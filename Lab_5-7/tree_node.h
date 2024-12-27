// tree_node.h
#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <memory>
#include <unistd.h> // Для pid_t

struct TreeNode {
    int id;
    pid_t pid;
    std::shared_ptr<TreeNode> left;
    std::shared_ptr<TreeNode> right;
    int parent_id;

    TreeNode(int node_id, pid_t node_pid, int parent = -1)
        : id(node_id), pid(node_pid), left(nullptr), right(nullptr), parent_id(parent) {}
};

#endif // TREE_NODE_H
