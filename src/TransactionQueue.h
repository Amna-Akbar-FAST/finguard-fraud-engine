#pragma once

#include "FinGuard.h"

// FIFO transaction buffer implemented with a singly linked list.
class TransactionQueue
{
private:
    struct Node
    {
        Transaction transaction;
        Node* next;

        explicit Node(const Transaction& txn);
    };

    Node* frontNode;
    Node* rearNode;
    int queueSize;

public:
    TransactionQueue();
    ~TransactionQueue();

    TransactionQueue(const TransactionQueue&) = delete;
    TransactionQueue& operator=(const TransactionQueue&) = delete;

    void enqueue(const Transaction& txn);

    // Returns false and leaves the queue unchanged when it is empty.
    bool dequeue(Transaction& txn);

    // Convenience overload. On an empty queue, returns a default Transaction
    // with flagReason set to "EMPTY_QUEUE".
    Transaction dequeue();

    bool peek(Transaction& txn) const;
    Transaction peek() const;

    bool isEmpty() const;
    int size() const;
    void displayQueue() const;
};
