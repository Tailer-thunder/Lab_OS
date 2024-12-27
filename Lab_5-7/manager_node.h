// manager_node.h
#ifndef MANAGER_NODE_H
#define MANAGER_NODE_H

#include "tree_node.h"
#include <map>
#include <mutex>
#include <string>
#include <unistd.h> // Для pid_t
#include <zmq.hpp>
#include <functional>

class ManagerNode {
public:
    ManagerNode();
    ~ManagerNode();

    // Метод для создания нового вычислительного узла
    void create_node(int id, int parent_id = -1);

    // Метод для выполнения команды на вычислительном узле
    void exec_command(int id, const std::string& command);

    // Метод для проверки доступности узла
    void ping_node(int id);

    // Метод для остановки всех узлов
    void stop();

private:
    std::shared_ptr<TreeNode> root_;
    std::map<int, pid_t> node_pids_; // Соответствие ID узла и PID процесса
    std::mutex mutex_;               // Мьютекс для синхронизации доступа

    // Вспомогательные методы для работы с деревом
    std::shared_ptr<TreeNode> find_node(int id);
    bool insert_node(std::shared_ptr<TreeNode> parent, std::shared_ptr<TreeNode> new_node);
};

#endif // MANAGER_NODE_H
