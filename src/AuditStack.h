#pragma once

#include "FinGuard.h"

// One decision saved in the analyst's audit history.
struct AuditAction
{
    Transaction transaction;
    string action;

    AuditAction();
    AuditAction(const Transaction& txn, const string& actionName);
};

// LIFO audit history implemented with a dynamically resized array.
class AuditStack
{
private:
    AuditAction* stackArray;
    int topIndex;
    int capacity;

    void resize();

public:
    explicit AuditStack(int initialCapacity = 10);
    ~AuditStack();

    AuditStack(const AuditStack&) = delete;
    AuditStack& operator=(const AuditStack&) = delete;

    void push(const AuditAction& action);
    void push(const Transaction& txn, const string& actionName);

    // Safe pop: returns false when the stack is empty.
    bool pop(AuditAction& action);

    // Convenience pop: returns an action named "EMPTY_STACK" when empty.
    AuditAction pop();

    bool top(AuditAction& action) const;
    AuditAction top() const;

    bool isEmpty() const;
    int size() const;
    void displayHistory() const;
};
