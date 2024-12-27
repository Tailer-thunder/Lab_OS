// computing_node.h
#ifndef COMPUTING_NODE_H
#define COMPUTING_NODE_H

#include <zmq.hpp>
#include <string>
#include <map>
#include <atomic>
#include <thread>

class ComputingNode {
public:
    ComputingNode(int id);
    void run();
    void stop();

private:
    int id_; // Идентификатор узла
    zmq::context_t context_;
    zmq::socket_t socket_;
    std::map<std::string, int> data_; // Локальное хранилище
    std::atomic<bool> running_;
};

#endif // COMPUTING_NODE_H
