#include "TransactionBST.h"

#include <iostream>

using namespace std;

TransactionBST::Node::Node(const Transaction& txn)
    : transaction(txn), left(nullptr), right(nullptr)
{
}

TransactionBST::TransactionBST()
    : root(nullptr), nodeCount(0)
{
}

TransactionBST::~TransactionBST()
{
    destroyTree(root);
    root = nullptr;
    nodeCount = 0;
}

void TransactionBST::destroyTree(Node* current)
{
    if (current == nullptr)
    {
        return;
    }

    destroyTree(current->left);
    destroyTree(current->right);
    delete current;
}

void TransactionBST::insertNode(Node*& current, const Transaction& txn)
{
    if (current == nullptr)
    {
        current = new Node(txn);
        return;
    }

    if (txn.amount < current->transaction.amount)
    {
        insertNode(current->left, txn);
    }
    else
    {
        // Equal amounts are also kept; they go to the right subtree.
        insertNode(current->right, txn);
    }
}

void TransactionBST::insert(const Transaction& txn)
{
    insertNode(root, txn);
    nodeCount++;
}

bool TransactionBST::searchNode(const Node* current, double amount) const
{
    if (current == nullptr)
    {
        return false;
    }

    if (amount == current->transaction.amount)
    {
        return true;
    }

    if (amount < current->transaction.amount)
    {
        return searchNode(current->left, amount);
    }

    return searchNode(current->right, amount);
}

bool TransactionBST::search(double amount) const
{
    return searchNode(root, amount);
}

void TransactionBST::inOrderTraversal(const Node* current) const
{
    if (current == nullptr)
    {
        return;
    }

    inOrderTraversal(current->left);
    current->transaction.display();
    inOrderTraversal(current->right);
}

void TransactionBST::inOrder() const
{
    if (isEmpty())
    {
        cout << "BST is empty. No transactions to display.\n";
        return;
    }

    cout << "\n========== TRANSACTIONS IN ASCENDING AMOUNT ORDER ==========\n";
    inOrderTraversal(root);
    cout << "===========================================================\n";
}

void TransactionBST::rangeQueryTraversal(const Node* current, double minAmount, double maxAmount) const
{
    if (current == nullptr)
    {
        return;
    }

    // Visit the left side only when it may contain an amount >= minAmount.
    if (current->transaction.amount > minAmount)
    {
        rangeQueryTraversal(current->left, minAmount, maxAmount);
    }

    if (current->transaction.amount >= minAmount && current->transaction.amount <= maxAmount)
    {
        current->transaction.display();
    }

    // Visit the right side only when it may contain an amount <= maxAmount.
    if (current->transaction.amount <= maxAmount)
    {
        rangeQueryTraversal(current->right, minAmount, maxAmount);
    }
}

void TransactionBST::rangeQuery(double minAmount, double maxAmount) const
{
    if (minAmount > maxAmount)
    {
        cout << "Invalid range: minimum amount cannot be greater than maximum amount.\n";
        return;
    }

    if (isEmpty())
    {
        cout << "BST is empty. No transactions found.\n";
        return;
    }

    cout << "\nTransactions with amount from $" << minAmount
         << " to $" << maxAmount << ":\n";
    rangeQueryTraversal(root, minAmount, maxAmount);
}

bool TransactionBST::isEmpty() const
{
    return root == nullptr;
}

int TransactionBST::size() const
{
    return nodeCount;
}
