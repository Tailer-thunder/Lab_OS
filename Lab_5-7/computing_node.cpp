// computing_node.cpp
#include "computing_node.h"
#include <iostream>
#include <sstream>
#include <unistd.h> // Для getpid()

ComputingNode::ComputingNode(int id)
    : id_(id), context_(1), socket_(context_, zmq::socket_type::rep), running_(true) {
    std::ostringstream endpoint;
    endpoint << "tcp://*:" << (5555 + id); // Уникальный порт на основе ID
    socket_.bind(endpoint.str());
    std::cout << "Node " << id_ << " started at " << endpoint.str() << "\n";
}

void ComputingNode::run() {
    while (running_) {
        zmq::message_t request;
        zmq::recv_result_t result = socket_.recv(request, zmq::recv_flags::none);
        if (!result) {
            std::cerr << "Error: Failed to receive request\n";
            continue;
        }

        std::string message(static_cast<char*>(request.data()), request.size());
        std::cout << "Node " << id_ << " received: " << message << "\n";

        std::istringstream iss(message);
        std::string command;
        iss >> command;

        if (command == "exec") {
            std::string name;
            iss >> name;
            if (name.empty()) {
                std::cout << "Node " << id_ << " received invalid exec command\n";
                std::string response = "Error: Invalid exec command";
                socket_.send(zmq::buffer(response), zmq::send_flags::none);
                continue;
            }

            int value = 0;
            if (iss >> value) {
                // Set command
                data_[name] = value;
                std::cout << "Node " << id_ << " stored: " << name << " = " << value << "\n";
                std::string response = "Ok:" + std::to_string(id_);
                socket_.send(zmq::buffer(response), zmq::send_flags::none);
            } else {
                // Get command
                auto it = data_.find(name);
                if (it != data_.end()) {
                    std::cout << "Node " << id_ << " returned value for " << name << ": " << it->second << "\n";
                    std::string response = "Ok:" + std::to_string(id_) + ": " + std::to_string(it->second);
                    socket_.send(zmq::buffer(response), zmq::send_flags::none);
                } else {
                    std::cout << "Node " << id_ << " could not find key: " << name << "\n";
                    std::string response = "Ok:" + std::to_string(id_) + ": '" + name + "' not found";
                    socket_.send(zmq::buffer(response), zmq::send_flags::none);
                }
            }
        } else if (command == "ping") {
            // Handle ping command
            std::string response = "Ok:1"; // Node is alive
            socket_.send(zmq::buffer(response), zmq::send_flags::none);
        } else {
            std::cout << "Node " << id_ << " received unknown command: " << command << "\n";
            std::string response = "Error: Unknown command";
            socket_.send(zmq::buffer(response), zmq::send_flags::none);
        }
    }
}

void ComputingNode::stop() {
    running_ = false;
    socket_.close();
}

// Main function for computing node
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: computing_node <id>\n";
        return 1;
    }

    int id = std::stoi(argv[1]);
    ComputingNode node(id);
    node.run();

    return 0;
}
