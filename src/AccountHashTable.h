#pragma once

#include "FinGuard.h"

// Account baseline information used by the fraud-analysis pipeline.


// ACCOUNT PROFILE STRUCT
// Stores each user's historical baseline behavior inside the AccountHashTable.
// Used by the FraudRuleEngine (Phase 3.1) to detect deviations in spend,
// location, and transaction frequency.


struct AccountProfile
{
    // Maximum number of recent timestamps tracked per account for Rule C
    // (avoids STL containers while supporting sliding-window velocity checks)
    static const int MAX_RECENT_TIMESTAMPS = 20;
    string accountID;           // Unique account identifier (e.g., "ACC101")
    double avgSpend;            // Running average of cleared transaction amounts
    string homeCity;            // Primary/initial city observed for the account
    int txnCount;               // Total number of cleared transactions used in avgSpend
    int lastActiveTimestamp;    // Timestamp (in minutes) of the most recent transaction

    // Fixed-size array storing recent transaction timestamps in chronological order
    // to check how many transactions occur within a 5-minute window (Rule C)
    int recentTimestamps[MAX_RECENT_TIMESTAMPS];
    int recentTimestampCount;
    AccountProfile();
    AccountProfile(const string& id, double averageSpend, const string& city,
                   int transactionCount, int lastActive = -1);
};

// Hash table implemented manually with separate chaining (linked lists).

// ACCOUNT HASH TABLE (SEPARATE CHAINING)
// Provides O(1) average-time lookup and updates for AccountProfile records.
class AccountHashTable
{
private:
    struct Node
    {
        AccountProfile profile;
        Node* next;

        explicit Node(const AccountProfile& account);
    };

    Node** buckets;
    int tableCapacity;
    int entryCount;

    void clear();

public:
    explicit AccountHashTable(int initialCapacity = 101);
    ~AccountHashTable();

    AccountHashTable(const AccountHashTable&) = delete;
    AccountHashTable& operator=(const AccountHashTable&) = delete;

    // Public because it is one of the required Hash Table operations.
    int hashFunc(const string& accID) const;

    void insertOrUpdate(const AccountProfile& account);
    void insertOrUpdate(const string& accountID, double avgSpend,
                        const string& homeCity, int txnCount,
                        int lastActiveTimestamp = -1);

    // Safe lookup: returns false when accountID is not found.
    bool getProfile(const string& accountID, AccountProfile& result) const;

    // Convenience lookup: returns a default profile when not found.
    AccountProfile getProfile(const string& accountID) const;

    // Update the running average after a cleared transaction.
    // Returns false when the account does not already exist.
    bool updateAverageSpend(const string& accountID, double clearedAmount);
    bool updateLastActiveTimestamp(const string& accountID, int timestamp);

    bool contains(const string& accountID) const;
    bool isEmpty() const;
    int size() const;

    // PHASE 3.1 VELOCITY TRACKING HELPERS (RULE C)

    // Appends a new transaction timestamp to the account's sliding window.
    // If the array is full, shifts older entries left to retain the newest ones.


    bool recordTransactionTimestamp(const string& accountID, int timestamp);
    int countTransactionsInWindow(const string& accountID, int currentTimestamp, int windowMinutes) const;
    void displayTable() const;
};
