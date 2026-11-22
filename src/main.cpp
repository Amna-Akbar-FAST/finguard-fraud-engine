#include "FinGuard.h"
#include "TransactionQueue.h"
#include "AccountHashTable.h"
#include "AuditStack.h"
#include "TransactionBST.h"
#include "AlertMaxHeap.h"
#include "FraudRuleEngine.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

using namespace std;

// HELPER: SEED ACCOUNT BASELINE PROFILES IN HASH TABLE
// Registers each unique accountID using its first observed transaction as its
// initial average-spend baseline and registered home city.

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
                txn.amount,     // Initial baseline spend
                txn.location,   // Registered home city
                1,              // Starting baseline count
                -1              // Updated as transactions are processed
            );
        }
    }
}


// [1] INGEST CSV INTO QUEUE
// Reads transactions from CSV, enqueues them into the FIFO TransactionQueue,
// and seeds initial user profiles in AccountHashTable.

bool ingestCSVIntoQueue(TransactionQueue& transactionQueue,
    AccountHashTable& accountTable,
    bool& isIngested)
{
    if (isIngested)
    {
        cout << "\n[INFO] CSV dataset has already been ingested into the pipeline.\n"
            << "Pending Queue Records: " << transactionQueue.size() << '\n';
        return true;
    }

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
        cout << "\n[ERROR] Could not load 'data/transactions.csv'. Verify file path.\n";
        delete[] transactions;
        return false;
    }

    displayTransactions(transactions, loadedCount);

    for (int i = 0; i < loadedCount; i++)
    {
        transactionQueue.enqueue(transactions[i]);
    }

    initializeAccountProfiles(transactions, loadedCount, accountTable);

    delete[] transactions;
    transactions = nullptr;
    isIngested = true;

    cout << "\n[SUCCESS] Ingested " << transactionQueue.size()
        << " transactions into FIFO Queue and seeded "
        << accountTable.size() << " account profiles in Hash Table.\n";
    return true;
}


// SINGLE TRANSACTION SCORING, ROUTING & TRIAGE (PHASE 3.1 & 3.2)


bool processSingleTransaction(TransactionQueue& transactionQueue,
    AccountHashTable& accountTable,
    AuditStack& auditStack,
    TransactionBST& transactionBST,
    AlertMaxHeap& alertHeap,
    bool verbose = true)
{
    Transaction txn;

    if (!transactionQueue.dequeue(txn))
    {
        if (verbose)
        {
            cout << "\n[INFO] No pending transactions in the Queue.\n";
        }
        return false;
    }

    AccountProfile profileBefore;
    bool hasProfile = accountTable.getProfile(txn.accountID, profileBefore);

    if (verbose && hasProfile)
    {
        cout << "\n------------------------------------------------------------\n"
            << "Processing " << txn.transactionID
            << " | Account: " << txn.accountID
            << " | Amount: $" << fixed << setprecision(2) << txn.amount
            << " | Time: " << txn.timestamp
            << " | Loc: " << txn.location
            << " | Merchant: " << txn.merchantType << "\n"
            << "Baseline -> Avg Spend: $" << profileBefore.avgSpend
            << " | Home City: " << profileBefore.homeCity
            << " | Txn Count: " << profileBefore.txnCount << '\n';
    }

    // Phase 3.1: Evaluate heuristic fraud rules (Rules A, B, C, D)
    FraudRuleEngine::evaluateTransaction(txn, accountTable);

    // Phase 3.2: Triage based on riskScore threshold (>= 50)
    if (txn.riskScore >= 50)
    {
        alertHeap.insert(txn);
        auditStack.push(txn, "FLAGGED");

        if (verbose)
        {
            cout << "[ALERT - FLAGGED] Risk Score: " << txn.riskScore
                << " | Reason(s): " << txn.flagReason
                << " -> Inserted into Alert Max-Heap.\n";
        }
    }
    else
    {
        if (txn.flagReason == "NONE" || txn.flagReason.empty())
        {
            txn.flagReason = "CLEARED";
        }

        // Update running average spend in Hash Table for cleared transactions
        if (hasProfile && profileBefore.lastActiveTimestamp != -1)
        {
            accountTable.updateAverageSpend(txn.accountID, txn.amount);
        }

        auditStack.push(txn, "CLEARED");

        if (verbose)
        {
            cout << "[CLEARED] Risk Score: " << txn.riskScore
                << " | Notes: " << txn.flagReason
                << " -> Running average updated.\n";
        }
    }

    // Update account's latest active timestamp
    if (hasProfile && txn.timestamp > profileBefore.lastActiveTimestamp)
    {
        accountTable.updateLastActiveTimestamp(txn.accountID, txn.timestamp);
    }

    // Archive every processed transaction in BST for historical auditing
    transactionBST.insert(txn);

    if (verbose)
    {
        cout << "Archived in BST (Total BST records: " << transactionBST.size() << ").\n";
    }

    return true;
}

// [2] PROCESS NEXT / ALL TRANSACTIONS
// Sub-menu allowing step-by-step single transaction evaluation or full batch
// processing of the entire FIFO Queue.


void handleProcessTransactions(TransactionQueue& transactionQueue,
    AccountHashTable& accountTable,
    AuditStack& auditStack,
    TransactionBST& transactionBST,
    AlertMaxHeap& alertHeap)
{
    if (transactionQueue.isEmpty())
    {
        cout << "\n[INFO] Transaction Queue is empty. Use [1] to ingest CSV data first.\n";
        return;
    }

    int mode = 0;
    cout << "\n--- Process Transactions ---\n"
        << "  1. Process Next Single Transaction\n"
        << "  2. Process ALL Pending Transactions (" << transactionQueue.size() << " remaining)\n"
        << "Select mode (1-2): ";

    if (!(cin >> mode) || (mode != 1 && mode != 2))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid selection. Returning to main menu.\n";
        return;
    }

    if (mode == 1)
    {
        processSingleTransaction(transactionQueue, accountTable, auditStack,
            transactionBST, alertHeap, true);
    }
    else
    {
        int processedCount = 0;
        int alertsBefore = alertHeap.size();

        while (!transactionQueue.isEmpty())
        {
            if (processSingleTransaction(transactionQueue, accountTable, auditStack,
                transactionBST, alertHeap, false))
            {
                processedCount++;
            }
        }

        int newAlerts = alertHeap.size() - alertsBefore;
        cout << "\n========== BATCH FRAUD TRIAGE COMPLETE ==========\n"
            << "Transactions Processed : " << processedCount << '\n'
            << "Flagged Fraud Alerts   : " << newAlerts << " (Score >= 50 -> Max-Heap)\n"
            << "Cleared Transactions   : " << (processedCount - newAlerts) << " (Score < 50 -> Avg Updated)\n"
            << "Total BST Records      : " << transactionBST.size() << '\n'
            << "=================================================\n";
    }
}


// [3] RESOLVE HIGHEST-RISK ALERT (POP MAX-HEAP)
// Extracts the top-priority alert from AlertMaxHeap and logs it on AuditStack.


void resolveHighestRiskAlert(AlertMaxHeap& alertHeap, AuditStack& auditStack)
{
    Transaction txn;

    if (!alertHeap.extractMax(txn))
    {
        cout << "\n[INFO] No active fraud alerts in the Max-Heap.\n";
        return;
    }

    cout << "\n[RESOLVED ALERT] Transaction: " << txn.transactionID
        << " | Account: " << txn.accountID
        << " | Amount: $" << fixed << setprecision(2) << txn.amount
        << " | Risk Score: " << txn.riskScore
        << " | Triggered Rules: " << txn.flagReason << '\n';

    txn.flagReason = "RESOLVED";
    auditStack.push(txn, "RESOLVED");
}


// [4] SEARCH AMOUNT RANGE (BST)
// Queries TransactionBST for all processed transactions within [min, max].


void searchAmountRangeBST(const TransactionBST& transactionBST)
{
    if (transactionBST.isEmpty())
    {
        cout << "\n[INFO] BST is empty. Process transactions first using [2].\n";
        return;
    }

    double minAmount = 0.0, maxAmount = 0.0;
    cout << "\nEnter minimum amount ($): ";
    if (!(cin >> minAmount))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid minimum amount.\n";
        return;
    }

    cout << "Enter maximum amount ($): ";
    if (!(cin >> maxAmount))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid maximum amount.\n";
        return;
    }

    transactionBST.rangeQuery(minAmount, maxAmount);
}


// [5] LOOKUP USER PROFILE (HASH TABLE)
// Performs O(1) lookup in AccountHashTable to inspect a user's baseline.


void lookupUserProfile(const AccountHashTable& accountTable)
{
    if (accountTable.isEmpty())
    {
        cout << "\n[INFO] Hash Table is empty. Ingest CSV data first using [1].\n";
        return;
    }

    string accountID;
    cout << "\nEnter Account ID (e.g., ACC101): ";
    cin >> accountID;

    AccountProfile profile;
    if (!accountTable.getProfile(accountID, profile))
    {
        cout << "[NOT FOUND] No profile exists for Account ID '" << accountID << "'.\n";
        return;
    }

    cout << "\n========== ACCOUNT PROFILE (HASH TABLE) ==========\n"
        << "Account ID            : " << profile.accountID << '\n'
        << "Average Spend         : $" << fixed << setprecision(2) << profile.avgSpend << '\n'
        << "Home City             : " << profile.homeCity << '\n'
        << "Cleared Txn Count     : " << profile.txnCount << '\n'
        << "Last Active Timestamp : " << profile.lastActiveTimestamp << '\n'
        << "==================================================\n";
}


// [6] UNDO LAST RESOLUTION (STACK)
// Pops the latest RESOLVED action from AuditStack and restores the alert back
// into AlertMaxHeap.


void undoLastResolution(AlertMaxHeap& alertHeap, AuditStack& auditStack)
{
    AuditAction latestAction;

    if (!auditStack.top(latestAction) || latestAction.action != "RESOLVED")
    {
        cout << "\n[INFO] The latest audit action is not a resolution. Nothing to undo.\n";
        return;
    }

    auditStack.pop(latestAction);
    latestAction.transaction.flagReason = "RESTORED_FROM_UNDO";
    alertHeap.insert(latestAction.transaction);
    auditStack.push(latestAction.transaction, "UNDO_RESOLUTION");

    cout << "\n[UNDO SUCCESS] Restored alert " << latestAction.transaction.transactionID
        << " (Risk Score: " << latestAction.transaction.riskScore
        << ") back to the Alert Max-Heap.\n";
}


// [7] SYSTEM DATA STRUCTURE INSPECTOR (DIAGNOSTIC VIEW)
// Allows viewing the internal state of all 5 Phase 2 data structures.


void inspectDataStructures(const TransactionQueue& transactionQueue,
    const AlertMaxHeap& alertHeap,
    const AuditStack& auditStack,
    const TransactionBST& transactionBST,
    const AccountHashTable& accountTable)
{
    int subChoice = 0;
    cout << "\n--- Inspect Data Structures ---\n"
        << "  1. View Pending Transaction Queue (FIFO)\n"
        << "  2. View Active Fraud Alerts (Max-Heap)\n"
        << "  3. View Analyst Audit History (Stack)\n"
        << "  4. View All Processed Transactions Sorted by Amount (BST In-Order)\n"
        << "  5. View All Account Profiles (Hash Table)\n"
        << "Select view (1-5): ";

    if (!(cin >> subChoice))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid selection.\n";
        return;
    }

    switch (subChoice)
    {
    case 1:
        transactionQueue.displayQueue();
        break;
    case 2:
        alertHeap.displayHeap();
        break;
    case 3:
        auditStack.displayHistory();
        break;
    case 4:
        transactionBST.inOrder();
        break;
    case 5:
        accountTable.displayTable();
        break;
    default:
        cout << "Invalid option.\n";
    }
}


// MAIN CONSOLE APPLICATION LOOP (PHASE 3.3)


int main()
{
    TransactionQueue transactionQueue;
    AccountHashTable accountTable;
    AuditStack auditStack;
    TransactionBST transactionBST;
    AlertMaxHeap alertHeap;

    bool isIngested = false;
    int choice = -1;

    cout << "====================================================================\n"
        << "        FINGUARD: REAL-TIME FINANCIAL FRAUD DETECTION ENGINE        \n"
        << "====================================================================\n";

    while (choice != 0)
    {
        cout << "\n==================== FINGUARD MAIN MENU ====================\n"
            << "  [1] Ingest CSV into Queue"
            << (isIngested ? " [LOADED]" : " [PENDING]") << '\n'
            << "  [2] Process Next / All Transactions (Queue: "
            << transactionQueue.size() << ")\n"
            << "  [3] Resolve Highest-Risk Alert (Pop Max-Heap | Alerts: "
            << alertHeap.size() << ")\n"
            << "  [4] Search Amount Range (BST | Archived: "
            << transactionBST.size() << ")\n"
            << "  [5] Lookup User Profile (Hash Table | Accounts: "
            << accountTable.size() << ")\n"
            << "  [6] Undo Last Resolution (Stack | Logs: "
            << auditStack.size() << ")\n"
            << "  [7] Inspect Data Structures (Queue / Heap / Stack / BST / Table)\n"
            << "  [0] Exit\n"
            << "============================================================\n"
            << "Select an option: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number from 0 to 7.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
            ingestCSVIntoQueue(transactionQueue, accountTable, isIngested);
            break;

        case 2:
            handleProcessTransactions(transactionQueue, accountTable, auditStack,
                transactionBST, alertHeap);
            break;

        case 3:
            resolveHighestRiskAlert(alertHeap, auditStack);
            break;

        case 4:
            searchAmountRangeBST(transactionBST);
            break;

        case 5:
            lookupUserProfile(accountTable);
            break;

        case 6:
            undoLastResolution(alertHeap, auditStack);
            break;

        case 7:
            inspectDataStructures(transactionQueue, alertHeap, auditStack,
                transactionBST, accountTable);
            break;

        case 0:
            cout << "\nExiting FinGuard. Goodbye!\n";
            break;

        default:
            cout << "Invalid option. Please choose 0-7.\n";
        }
    }

    return 0;
}