#include "TransactionQueue.h"

#include <iostream>

using namespace std;

TransactionQueue::Node::Node(const Transaction& txn)
    : transaction(txn), next(nullptr)
{
}

TransactionQueue::TransactionQueue()
    : frontNode(nullptr), rearNode(nullptr), queueSize(0)
{
}

TransactionQueue::~TransactionQueue()
{
    while (frontNode != nullptr)
    {
        Node* current = frontNode;
        frontNode = frontNode->next;
        delete current;
    }

    rearNode = nullptr;
    queueSize = 0;
}

void TransactionQueue::enqueue(const Transaction& txn)
{
    Node* newNode = new Node(txn);

    if (isEmpty())
    {
        frontNode = newNode;
        rearNode = newNode;
    }
    else
    {
        rearNode->next = newNode;
        rearNode = newNode;
    }

    queueSize++;
}

bool TransactionQueue::dequeue(Transaction& txn)
{
    if (isEmpty())
    {
        return false;
    }

    Node* oldFront = frontNode;
    txn = oldFront->transaction;
    frontNode = frontNode->next;
    delete oldFront;
    queueSize--;

    if (frontNode == nullptr)
    {
        rearNode = nullptr;
    }

    return true;
}

Transaction TransactionQueue::dequeue()
{
    Transaction txn;

    if (!dequeue(txn))
    {
        txn.flagReason = "EMPTY_QUEUE";
    }

    return txn;
}

bool TransactionQueue::peek(Transaction& txn) const
{
    if (isEmpty())
    {
        return false;
    }

    txn = frontNode->transaction;
    return true;
}

Transaction TransactionQueue::peek() const
{
    Transaction txn;

    if (!peek(txn))
    {
        txn.flagReason = "EMPTY_QUEUE";
    }

    return txn;
}

bool TransactionQueue::isEmpty() const
{
    return frontNode == nullptr;
}

int TransactionQueue::size() const
{
    return queueSize;
}

void TransactionQueue::displayQueue() const
{
    if (isEmpty())
    {
        cout << "Transaction Queue is empty.\n";
        return;
    }

    cout << "\n========== TRANSACTION QUEUE (FRONT TO REAR) ==========\n";
    const Node* current = frontNode;

    while (current != nullptr)
    {
        current->transaction.display();
        current = current->next;
    }

    cout << "=======================================================\n";
}
