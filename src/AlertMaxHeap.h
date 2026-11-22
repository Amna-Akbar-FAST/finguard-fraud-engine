#pragma once

#include "FinGuard.h"

// Max-Heap for active fraud alerts, ordered by Transaction::riskScore.
// The highest riskScore is always stored at index 0.
class AlertMaxHeap
{
private:
    Transaction* heapArray;
    int heapSize;
    int capacity;

    void resize();

public:
    explicit AlertMaxHeap(int initialCapacity = 10);
    ~AlertMaxHeap();

    // A heap owns its dynamic array, so copying is disabled.
    AlertMaxHeap(const AlertMaxHeap&) = delete;
    AlertMaxHeap& operator=(const AlertMaxHeap&) = delete;

    void insert(const Transaction& txn);

    // Required Phase 2 operation. On an empty heap, returns a Transaction
    // whose riskScore is -1 and flagReason is "EMPTY_HEAP".
    Transaction extractMax();

    // Safer overload: returns false when the heap is empty.
    bool extractMax(Transaction& highestRiskTransaction);

    Transaction peekMax() const;
    bool isEmpty() const;
    int size() const;

    void heapifyUp(int index);
    void heapifyDown(int index);
    void displayHeap() const;
};
