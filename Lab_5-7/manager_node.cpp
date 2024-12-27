// manager_node.cpp
#include "manager_node.h"
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <thread>

// Конструктор
ManagerNode::ManagerNode() {
    // Инициализация корня дерева, если необходимо
    root_ = nullptr;
}

// Деструктор
ManagerNode::~ManagerNode() {
    stop(); // Остановить все узлы при уничтожении объекта
}

// Метод для поиска узла по ID
std::shared_ptr<TreeNode> ManagerNode::find_node(int id) {
    if (!root_) return nullptr;
    std::shared_ptr<TreeNode> current = root_;
    while (current) {
        if (id == current->id) return current;
        else if (id < current->id) current = current->left;
        else current = current->right;
    }
    return nullptr;
}

// Метод для вставки узла в дерево (уровневый обход для балансировки)
bool ManagerNode::insert_node(std::shared_ptr<TreeNode> parent, std::shared_ptr<TreeNode> new_node) {
    if (!parent) return false;

    std::queue<std::shared_ptr<TreeNode>> q;
    q.push(parent);

    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        if (!current->left) {
            current->left = new_node;
            return true;
        } else {
            q.push(current->left);
        }

        if (!current->right) {
            current->right = new_node;
            return true;
        } else {
            q.push(current->right);
        }
    }
    return false;
}

// Метод для создания нового вычислительного узла
void ManagerNode::create_node(int id, int parent_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Проверка на существование узла с таким же ID
    if (node_pids_.find(id) != node_pids_.end()) {
        std::cout << "Error: Already exists\n";
        return;
    }

    // Если указан parent_id, проверяем его существование и доступность
    if (parent_id != -1) {
        auto parent_node = find_node(parent_id);
        if (!parent_node) {
            std::cout << "Error: Parent not found\n";
            return;
        }
        // Проверка доступности родительского узла
        if (kill(parent_node->pid, 0) != 0) {
            std::cout << "Error: Parent is unavailable\n";
            return;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cout << "Error: Failed to create node\n";
        return;
    }

    if (pid == 0) {
        // Дочерний процесс: запуск вычислительного узла
        // Предполагается, что вычислительный узел принимает ID как аргумент
        std::string id_str = std::to_string(id);
        execl("./computing_node", "computing_node", id_str.c_str(), (char*)nullptr);
        // Если execl возвращает, произошла ошибка
        std::cerr << "Error: execl failed for computing_node with ID " << id << "\n";
        exit(1);
    } else {
        // Родительский процесс: сохраняем PID
        node_pids_[id] = pid;
        auto new_node = std::make_shared<TreeNode>(id, pid, parent_id);
        if (!root_) {
            root_ = new_node;
        } else {
            if (parent_id != -1) {
                auto parent_node = find_node(parent_id);
                if (parent_node) {
                    insert_node(parent_node, new_node);
                } else {
                    // Это не должно произойти, так как мы уже проверили существование родителя
                    // Вставляем в корень
                    insert_node(root_, new_node);
                }
            } else {
                // Если parent_id == -1, вставляем в первое доступное место для балансировки
                insert_node(root_, new_node);
            }
        }
        std::cout << "Ok: " << pid << "\n";
    }
}

// Метод для выполнения команды на вычислительном узле
void ManagerNode::exec_command(int id, const std::string& command) {
    std::thread([this, id, command]() {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = node_pids_.find(id);
        if (it == node_pids_.end()) {
            std::cout << "Error:" << id << ": Not found\n";
            return;
        }

        pid_t pid = it->second;
        // Проверка доступности процесса
        if (kill(pid, 0) != 0) {
            std::cout << "Error:" << id << ": Node is unavailable\n";
            return;
        }

        // Использование ZeroMQ для отправки команды вычислительному узлу
        zmq::context_t context(1);
        zmq::socket_t socket(context, zmq::socket_type::req);

        std::ostringstream endpoint;
        endpoint << "tcp://localhost:" << (5555 + id);
        try {
            socket.connect(endpoint.str());
        } catch (const zmq::error_t& e) {
            std::cout << "Error:" << id << ": Failed to connect to node\n";
            return;
        }

        // Добавляем "exec " к команде
        std::string full_command = "exec " + command;
        zmq::message_t request(full_command.size());
        memcpy(request.data(), full_command.c_str(), full_command.size());

        try {
            socket.send(request, zmq::send_flags::none);
        } catch (const zmq::error_t& e) {
            std::cout << "Error:" << id << ": Failed to send command\n";
            return;
        }

        zmq::message_t reply;
        zmq::recv_result_t result;
        try {
            result = socket.recv(reply, zmq::recv_flags::none);
        } catch (const zmq::error_t& e) {
            std::cout << "Error:" << id << ": Failed to receive response\n";
            return;
        }

        if (result) {
            std::string response(static_cast<char*>(reply.data()), reply.size());
            std::cout << response << "\n";
        } else {
            std::cout << "Error:" << id << ": Failed to receive response\n";
        }
    }).detach(); // Отсоединяем поток для асинхронного выполнения
}

// Метод для проверки доступности узла
void ManagerNode::ping_node(int id) {
    std::thread([this, id]() {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = node_pids_.find(id);
        if (it == node_pids_.end()) {
            std::cout << "Error: Not found\n";
            return;
        }

        pid_t pid = it->second;
        if (kill(pid, 0) == 0) {
            // Узел жив: отправляем ping через ZeroMQ
            zmq::context_t context(1);
            zmq::socket_t socket(context, zmq::socket_type::req);

            std::ostringstream endpoint;
            endpoint << "tcp://localhost:" << (5555 + id);
            try {
                socket.connect(endpoint.str());
            } catch (const zmq::error_t& e) {
                std::cout << "Ok: 0\n"; // Узел недоступен
                return;
            }

            std::string ping_cmd = "ping";
            zmq::message_t request(ping_cmd.size());
            memcpy(request.data(), ping_cmd.c_str(), ping_cmd.size());

            try {
                socket.send(request, zmq::send_flags::none);
            } catch (const zmq::error_t& e) {
                std::cout << "Ok: 0\n"; // Узел недоступен
                return;
            }

            zmq::message_t reply;
            zmq::recv_result_t result;
            try {
                result = socket.recv(reply, zmq::recv_flags::none);
            } catch (const zmq::error_t& e) {
                std::cout << "Ok: 0\n"; // Узел недоступен
                return;
            }

            if (result) {
                std::string response(static_cast<char*>(reply.data()), reply.size());
                // Проверяем ответ на "Ok:1"
                if (response == "Ok:1") {
                    std::cout << "Ok: 1\n"; // Узел доступен
                } else {
                    std::cout << "Ok: 0\n"; // Узел недоступен
                }
            } else {
                std::cout << "Ok: 0\n"; // Узел недоступен
            }
        } else {
            std::cout << "Ok: 0\n"; // Узел недоступен
        }
    }).detach(); // Асинхронный запуск
}

// Метод для остановки всех узлов
void ManagerNode::stop() {
    std::lock_guard<std::mutex> lock(mutex_);

    // Рекурсивная функция для убийства узлов
    std::function<void(std::shared_ptr<TreeNode>)> kill_subtree = [&](std::shared_ptr<TreeNode> node) {
        if (!node) return;
        kill_subtree(node->left);
        kill_subtree(node->right);
        kill(node->pid, SIGKILL);
        std::cout << "Node " << node->id << " killed.\n";
    };

    kill_subtree(root_);
    node_pids_.clear();
    root_ = nullptr;
    std::cout << "All nodes stopped.\n";
}
