#pragma once

#include "FinGuard.h"

// Account baseline information used by the fraud-analysis pipeline.
struct AccountProfile
{
    string accountID;
    double avgSpend;
    string homeCity;
    int txnCount;
    int lastActiveTimestamp;

    AccountProfile();
    AccountProfile(const string& id, double averageSpend, const string& city,
                   int transactionCount, int lastActive = -1);
};

// Hash table implemented manually with separate chaining (linked lists).
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
    void displayTable() const;
};
