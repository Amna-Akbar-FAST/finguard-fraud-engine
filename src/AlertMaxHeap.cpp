#include "AlertMaxHeap.h"

#include <iostream>

using namespace std;

AlertMaxHeap::AlertMaxHeap(int initialCapacity)
    : heapArray(nullptr), heapSize(0), capacity(initialCapacity)
{
    if (capacity < 1)
    {
        capacity = 10;
    }

    heapArray = new Transaction[capacity];
}

AlertMaxHeap::~AlertMaxHeap()
{
    delete[] heapArray;
    heapArray = nullptr;
    heapSize = 0;
    capacity = 0;
}

void AlertMaxHeap::resize()
{
    int newCapacity = capacity * 2;
    Transaction* newArray = new Transaction[newCapacity];

    for (int i = 0; i < heapSize; i++)
    {
        newArray[i] = heapArray[i];
    }

    delete[] heapArray;
    heapArray = newArray;
    capacity = newCapacity;
}

void AlertMaxHeap::insert(const Transaction& txn)
{
    if (heapSize == capacity)
    {
        resize();
    }

    heapArray[heapSize] = txn;
    heapifyUp(heapSize);
    heapSize++;
}

void AlertMaxHeap::heapifyUp(int index)
{
    while (index > 0)
    {
        int parentIndex = (index - 1) / 2;

        if (heapArray[index].riskScore <= heapArray[parentIndex].riskScore)
        {
            break;
        }

        Transaction temp = heapArray[index];
        heapArray[index] = heapArray[parentIndex];
        heapArray[parentIndex] = temp;

        index = parentIndex;
    }
}

void AlertMaxHeap::heapifyDown(int index)
{
    while (true)
    {
        int leftChild = (2 * index) + 1;
        int rightChild = (2 * index) + 2;
        int largest = index;

        if (leftChild < heapSize &&
            heapArray[leftChild].riskScore > heapArray[largest].riskScore)
        {
            largest = leftChild;
        }

        if (rightChild < heapSize &&
            heapArray[rightChild].riskScore > heapArray[largest].riskScore)
        {
            largest = rightChild;
        }

        if (largest == index)
        {
            break;
        }

        Transaction temp = heapArray[index];
        heapArray[index] = heapArray[largest];
        heapArray[largest] = temp;

        index = largest;
    }
}

bool AlertMaxHeap::extractMax(Transaction& highestRiskTransaction)
{
    if (isEmpty())
    {
        return false;
    }

    highestRiskTransaction = heapArray[0];
    heapSize--;

    if (heapSize > 0)
    {
        heapArray[0] = heapArray[heapSize];
        heapifyDown(0);
    }

    return true;
}

Transaction AlertMaxHeap::extractMax()
{
    Transaction highestRiskTransaction;

    if (!extractMax(highestRiskTransaction))
    {
        highestRiskTransaction.riskScore = -1;
        highestRiskTransaction.flagReason = "EMPTY_HEAP";
    }

    return highestRiskTransaction;
}

Transaction AlertMaxHeap::peekMax() const
{
    if (isEmpty())
    {
        Transaction emptyTransaction;
        emptyTransaction.riskScore = -1;
        emptyTransaction.flagReason = "EMPTY_HEAP";
        return emptyTransaction;
    }

    return heapArray[0];
}

bool AlertMaxHeap::isEmpty() const
{
    return heapSize == 0;
}

int AlertMaxHeap::size() const
{
    return heapSize;
}

void AlertMaxHeap::displayHeap() const
{
    if (isEmpty())
    {
        cout << "Alert Max-Heap is empty.\n";
        return;
    }

    cout << "\n========== ACTIVE FRAUD ALERTS (HEAP ARRAY) ==========\n";
    for (int i = 0; i < heapSize; i++)
    {
        cout << "Index " << i << " | Risk Score: " << heapArray[i].riskScore
             << " | Transaction ID: " << heapArray[i].transactionID
             << " | Account ID: " << heapArray[i].accountID
             << " | Amount: $" << heapArray[i].amount << '\n';
    }
    cout << "======================================================\n";
}
