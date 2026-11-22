#pragma once

#include "FinGuard.h"

// Binary Search Tree for storing processed transactions by amount.
// Duplicate amounts are stored in the right subtree.
class TransactionBST
{
private:
    struct Node
    {
        Transaction transaction;
        Node* left;
        Node* right;

        explicit Node(const Transaction& txn);
    };

    Node* root;
    int nodeCount;

    void insertNode(Node*& current, const Transaction& txn);
    bool searchNode(const Node* current, double amount) const;
    void inOrderTraversal(const Node* current) const;
    void rangeQueryTraversal(const Node* current, double minAmount, double maxAmount) const;
    void destroyTree(Node* current);

public:
    TransactionBST();
    ~TransactionBST();

    // A tree owns its nodes, so copying is disabled to avoid double deletion.
    TransactionBST(const TransactionBST&) = delete;
    TransactionBST& operator=(const TransactionBST&) = delete;

    void insert(const Transaction& txn);
    bool search(double amount) const;
    void inOrder() const;
    void rangeQuery(double minAmount, double maxAmount) const;
    bool isEmpty() const;
    int size() const;
};
