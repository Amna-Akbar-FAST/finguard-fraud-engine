#include "FinGuard.h"
#include "TransactionQueue.h"
#include "AccountHashTable.h"
#include "AuditStack.h"
#include "TransactionBST.h"
#include "AlertMaxHeap.h"

#include <iostream>
#include <limits>
#include <string>

using namespace std;

// The CSV does not contain a separate account-profile file.
// For this first integration, the first observed transaction for each account
// is used as its starting average-spend sample and starting city.
void initializeAccountProfiles(const Transaction* transactions,
                               int count,
                               AccountHashTable& accountTable)
{
    for (int i = 0; i < count; i++)
    {
        const Transaction& txn = transactions[i];

        if (!accountTable.contains(txn.accountID))
        {
            accountTable.insertOrUpdate(
                txn.accountID,
                txn.amount,
                txn.location,
                1,
                txn.timestamp
            );
        }
    }
}

void loadQueue(const Transaction* transactions,
               int count,
               TransactionQueue& transactionQueue)
{
    for (int i = 0; i < count; i++)
    {
        transactionQueue.enqueue(transactions[i]);
    }
}

void processNextTransaction(TransactionQueue& transactionQueue,
                             AccountHashTable& accountTable,
                             AuditStack& auditStack,
                             TransactionBST& transactionBST,
                             AlertMaxHeap& alertHeap)
{
    Transaction txn;

    if (!transactionQueue.dequeue(txn))
    {
        cout << "No pending transactions in the Queue.\n";
        return;
    }

    AccountProfile profile;
    if (accountTable.getProfile(txn.accountID, profile))
    {
        cout << "\nAccount profile found: " << profile.accountID
             << " | Starting Avg Spend: $" << profile.avgSpend
             << " | Home City (first observed): " << profile.homeCity
             << " | Transaction Count: " << profile.txnCount
             << " | Last Active: " << profile.lastActiveTimestamp << '\n';
    }

    int enteredScore;
    while (true)
    {
        cout << "Enter a provisional risk score (0-100) for this transaction: ";
        if (cin >> enteredScore && enteredScore >= 0 && enteredScore <= 100)
        {
            break;
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid score. Enter a whole number from 0 to 100.\n";
    }

    txn.riskScore = enteredScore;

    if (txn.riskScore >= 50)
    {
        txn.flagReason = "FLAGGED_FOR_REVIEW";
        alertHeap.insert(txn);
        auditStack.push(txn, "FLAGGED");
        cout << "Transaction flagged and inserted into Alert Max-Heap.\n";
    }
    else
    {
        txn.flagReason = "CLEARED";
        auditStack.push(txn, "CLEARED");

        cout << "Transaction cleared.\n";
    }

    // Update last-active time for every processed transaction. Only cleared
    // transactions are included in the running average.
    if (accountTable.getProfile(txn.accountID, profile) &&
        txn.timestamp > profile.lastActiveTimestamp)
    {
        if (txn.riskScore < 50)
        {
            accountTable.updateAverageSpend(txn.accountID, txn.amount);
        }

        accountTable.updateLastActiveTimestamp(txn.accountID, txn.timestamp);
    }

    transactionBST.insert(txn);
    cout << "Transaction inserted into BST for amount-based history.\n";
}

void resolveHighestRiskAlert(AlertMaxHeap& alertHeap, AuditStack& auditStack)
{
    Transaction txn;

    if (!alertHeap.extractMax(txn))
    {
        cout << "No active fraud alerts to resolve.\n";
        return;
    }

    cout << "Resolved highest-risk alert: " << txn.transactionID
         << " | Account: " << txn.accountID
         << " | Amount: $" << txn.amount
         << " | Risk Score: " << txn.riskScore << '\n';

    txn.flagReason = "RESOLVED";
    auditStack.push(txn, "RESOLVED");
}

void lookupAccount(const AccountHashTable& accountTable)
{
    string accountID;
    cout << "Enter Account ID: ";
    cin >> accountID;

    AccountProfile profile;
    if (!accountTable.getProfile(accountID, profile))
    {
        cout << "Account not found.\n";
        return;
    }

    cout << "\nAccount ID: " << profile.accountID
         << "\nAverage Spend: $" << profile.avgSpend
         << "\nHome City (first observed): " << profile.homeCity
         << "\nTransaction Count: " << profile.txnCount
         << "\nLast Active Timestamp: " << profile.lastActiveTimestamp << '\n';
}

void undoLastResolution(AlertMaxHeap& alertHeap, AuditStack& auditStack)
{
    AuditAction latestAction;

    if (!auditStack.top(latestAction) || latestAction.action != "RESOLVED")
    {
        cout << "The latest audit action is not a resolution. Nothing to undo.\n";
        return;
    }

    auditStack.pop(latestAction);
    latestAction.transaction.flagReason = "FLAGGED_FOR_REVIEW";
    alertHeap.insert(latestAction.transaction);
    auditStack.push(latestAction.transaction, "UNDO_RESOLUTION");

    cout << "The alert has been restored to the Max-Heap.\n";
}

int main()
{
    const int MAX_RECORDS = 100;
    Transaction* transactions = new Transaction[MAX_RECORDS];

    string filePath = "data/transactions.csv";
    int loadedCount = loadTransactionsFromCSV(filePath, transactions, MAX_RECORDS);

    if (loadedCount <= 0)
    {
        filePath = "../data/transactions.csv";
        loadedCount = loadTransactionsFromCSV(filePath, transactions, MAX_RECORDS);
    }

    if (loadedCount <= 0)
    {
        filePath = "data/transactions.csv";
        loadedCount = loadTransactionsFromCSV(filePath, transactions, MAX_RECORDS);
    }

    if (loadedCount <= 0)
    {
        cout << "No transaction loaded. Verify data/transactions.csv path.\n";
        delete[] transactions;
        return 1;
    }

    cout << "\nTotal Records Loaded: " << loadedCount << '\n';
    displayTransactions(transactions, loadedCount);

    TransactionQueue transactionQueue;
    AccountHashTable accountTable;
    AuditStack auditStack;
    TransactionBST transactionBST;
    AlertMaxHeap alertHeap;

    loadQueue(transactions, loadedCount, transactionQueue);
    initializeAccountProfiles(transactions, loadedCount, accountTable);

    cout << "\nPhase 2 structures initialized successfully."
         << "\nQueue records: " << transactionQueue.size()
         << "\nAccount profiles: " << accountTable.size() << '\n';

    delete[] transactions;
    transactions = nullptr;

    int choice = -1;

    while (choice != 0)
    {
        cout << "\n========== FINGUARD DATA STRUCTURE MENU ==========\n"
             << "1. Display pending Queue\n"
             << "2. Process next transaction (enter provisional risk score)\n"
             << "3. Display active fraud alerts (Max-Heap)\n"
             << "4. Resolve highest-risk alert\n"
             << "5. Search transaction amount range (BST)\n"
             << "6. Lookup account profile (Hash Table)\n"
             << "7. Display audit history (Stack)\n"
             << "8. Undo last alert resolution\n"
             << "9. Display processed transactions in amount order (BST)\n"
             << "10. Display account Hash Table\n"
             << "0. Exit\n"
             << "Choose an option: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid menu number.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
            transactionQueue.displayQueue();
            break;

        case 2:
            processNextTransaction(transactionQueue, accountTable, auditStack,
                                   transactionBST, alertHeap);
            break;

        case 3:
            alertHeap.displayHeap();
            break;

        case 4:
            resolveHighestRiskAlert(alertHeap, auditStack);
            break;

        case 5:
        {
            double minAmount, maxAmount;
            cout << "Enter minimum amount: ";
            if (!(cin >> minAmount))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid minimum amount.\n";
                break;
            }

            cout << "Enter maximum amount: ";
            if (!(cin >> maxAmount))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid maximum amount.\n";
                break;
            }

            transactionBST.rangeQuery(minAmount, maxAmount);
            break;
        }

        case 6:
            lookupAccount(accountTable);
            break;

        case 7:
            auditStack.displayHistory();
            break;

        case 8:
            undoLastResolution(alertHeap, auditStack);
            break;

        case 9:
            transactionBST.inOrder();
            break;

        case 10:
            accountTable.displayTable();
            break;

        case 0:
            cout << "FinGuard closed.\n";
            break;

        default:
            cout << "Invalid option. Please choose 0-9.\n";
        }
    }

    return 0;
}
