// main.cpp
#include "manager_node.h"
#include <iostream>
#include <sstream>

int main() {
    ManagerNode manager;
    std::string command_line;

    // Вывод инструкций по запуску
    std::cout << "Distributed System Manager\n";
    std::cout << "Available commands:\n";
    std::cout << "  create <id> [parent] - Create a new computing node\n";
    std::cout << "  exec <id> <name> [value] - Execute command on computing node\n";
    std::cout << "  ping <id> - Check availability of a node\n";
    std::cout << "  stop - Stop all nodes and exit\n";
    std::cout << "Enter commands:\n";

    // Основной цикл для ввода команд пользователем
    while (std::getline(std::cin, command_line)) {
        std::istringstream iss(command_line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "create") {
            int id;
            int parent_id = -1;
            if (iss >> id) {
                if (iss >> parent_id) {
                    manager.create_node(id, parent_id);
                } else {
                    manager.create_node(id); // Без parent_id, по умолчанию -1
                }
            } else {
                std::cout << "Error: Invalid create command format\n";
            }
        } else if (cmd == "exec") {
            int id;
            iss >> id;
            std::string payload;
            std::getline(iss, payload);
            // Удаление ведущих пробелов
            size_t first = payload.find_first_not_of(" ");
            if (first != std::string::npos) {
                payload = payload.substr(first);
            }
            if (payload.empty()) {
                std::cout << "Error: Invalid exec command format\n";
                continue;
            }
            manager.exec_command(id, payload);
        } else if (cmd == "ping") {
            int id;
            if (iss >> id) {
                manager.ping_node(id);
            } else {
                std::cout << "Error: Invalid ping command format\n";
            }
        } else if (cmd == "stop") {
            manager.stop();
            break;
        } else {
            std::cout << "Unknown command\n";
        }
    }

    return 0;
}
