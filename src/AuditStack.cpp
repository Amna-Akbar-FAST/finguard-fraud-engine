#include "AuditStack.h"

#include <iostream>

using namespace std;

AuditAction::AuditAction()
    : transaction(), action("NONE")
{
}

AuditAction::AuditAction(const Transaction& txn, const string& actionName)
    : transaction(txn), action(actionName)
{
}

AuditStack::AuditStack(int initialCapacity)
    : stackArray(nullptr), topIndex(-1), capacity(initialCapacity)
{
    if (capacity < 1)
    {
        capacity = 10;
    }

    stackArray = new AuditAction[capacity];
}

AuditStack::~AuditStack()
{
    delete[] stackArray;
    stackArray = nullptr;
    topIndex = -1;
    capacity = 0;
}

void AuditStack::resize()
{
    int newCapacity = capacity * 2;
    AuditAction* newArray = new AuditAction[newCapacity];

    for (int i = 0; i <= topIndex; i++)
    {
        newArray[i] = stackArray[i];
    }

    delete[] stackArray;
    stackArray = newArray;
    capacity = newCapacity;
}

void AuditStack::push(const AuditAction& action)
{
    if (topIndex + 1 == capacity)
    {
        resize();
    }

    topIndex++;
    stackArray[topIndex] = action;
}

void AuditStack::push(const Transaction& txn, const string& actionName)
{
    AuditAction action(txn, actionName);
    push(action);
}

bool AuditStack::pop(AuditAction& action)
{
    if (isEmpty())
    {
        return false;
    }

    action = stackArray[topIndex];
    topIndex--;
    return true;
}

AuditAction AuditStack::pop()
{
    AuditAction action;

    if (!pop(action))
    {
        action.action = "EMPTY_STACK";
    }

    return action;
}

bool AuditStack::top(AuditAction& action) const
{
    if (isEmpty())
    {
        return false;
    }

    action = stackArray[topIndex];
    return true;
}

AuditAction AuditStack::top() const
{
    AuditAction action;

    if (!top(action))
    {
        action.action = "EMPTY_STACK";
    }

    return action;
}

bool AuditStack::isEmpty() const
{
    return topIndex == -1;
}

int AuditStack::size() const
{
    return topIndex + 1;
}

void AuditStack::displayHistory() const
{
    if (isEmpty())
    {
        cout << "Audit Stack is empty.\n";
        return;
    }

    cout << "\n========== AUDIT HISTORY (TOP TO BOTTOM) ==========" << endl;

    for (int i = topIndex; i >= 0; i--)
    {
        cout << "Action: " << stackArray[i].action
             << " | Transaction ID: " << stackArray[i].transaction.transactionID
             << " | Account ID: " << stackArray[i].transaction.accountID
             << " | Amount: $" << stackArray[i].transaction.amount
             << " | Risk Score: " << stackArray[i].transaction.riskScore
             << " | Reason: " << stackArray[i].transaction.flagReason
             << endl;
    }

    cout << "===================================================\n";
}
