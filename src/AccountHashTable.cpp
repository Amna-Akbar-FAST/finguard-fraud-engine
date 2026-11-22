#include "AccountHashTable.h"

#include <iostream>

using namespace std;

AccountProfile::AccountProfile()
    : accountID(""), avgSpend(0.0), homeCity(""), txnCount(0),
      lastActiveTimestamp(-1)
{
}

AccountProfile::AccountProfile(const string& id, double averageSpend,
                               const string& city, int transactionCount,
                               int lastActive)
    : accountID(id), avgSpend(averageSpend), homeCity(city),
      txnCount(transactionCount), lastActiveTimestamp(lastActive)
{
}

AccountHashTable::Node::Node(const AccountProfile& account)
    : profile(account), next(nullptr)
{
}

AccountHashTable::AccountHashTable(int initialCapacity)
    : buckets(nullptr), tableCapacity(initialCapacity), entryCount(0)
{
    if (tableCapacity < 1)
    {
        tableCapacity = 101;
    }

    buckets = new Node*[tableCapacity];

    for (int i = 0; i < tableCapacity; i++)
    {
        buckets[i] = nullptr;
    }
}

AccountHashTable::~AccountHashTable()
{
    clear();
    delete[] buckets;
    buckets = nullptr;
    tableCapacity = 0;
    entryCount = 0;
}

void AccountHashTable::clear()
{
    if (buckets == nullptr)
    {
        return;
    }

    for (int i = 0; i < tableCapacity; i++)
    {
        Node* current = buckets[i];

        while (current != nullptr)
        {
            Node* nextNode = current->next;
            delete current;
            current = nextNode;
        }

        buckets[i] = nullptr;
    }

    entryCount = 0;
}

int AccountHashTable::hashFunc(const string& accID) const
{
    unsigned long hashValue = 5381;

    for (size_t i = 0; i < accID.length(); i++)
    {
        hashValue = (hashValue * 33) + static_cast<unsigned char>(accID[i]);
    }

    return static_cast<int>(hashValue % static_cast<unsigned long>(tableCapacity));
}

void AccountHashTable::insertOrUpdate(const AccountProfile& account)
{
    if (account.accountID.empty())
    {
        return;
    }

    int index = hashFunc(account.accountID);
    Node* current = buckets[index];

    while (current != nullptr)
    {
        if (current->profile.accountID == account.accountID)
        {
            current->profile = account;
            return;
        }

        current = current->next;
    }

    Node* newNode = new Node(account);
    newNode->next = buckets[index];
    buckets[index] = newNode;
    entryCount++;
}

void AccountHashTable::insertOrUpdate(const string& accountID, double avgSpend,
                                      const string& homeCity, int txnCount,
                                      int lastActiveTimestamp)
{
    AccountProfile account(accountID, avgSpend, homeCity, txnCount,
                           lastActiveTimestamp);
    insertOrUpdate(account);
}

bool AccountHashTable::getProfile(const string& accountID,
                                  AccountProfile& result) const
{
    if (accountID.empty())
    {
        return false;
    }

    int index = hashFunc(accountID);
    const Node* current = buckets[index];

    while (current != nullptr)
    {
        if (current->profile.accountID == accountID)
        {
            result = current->profile;
            return true;
        }

        current = current->next;
    }

    return false;
}

AccountProfile AccountHashTable::getProfile(const string& accountID) const
{
    AccountProfile result;
    getProfile(accountID, result);
    return result;
}

bool AccountHashTable::updateAverageSpend(const string& accountID,
                                          double clearedAmount)
{
    if (clearedAmount < 0.0)
    {
        return false;
    }

    int index = hashFunc(accountID);
    Node* current = buckets[index];

    while (current != nullptr)
    {
        if (current->profile.accountID == accountID)
        {
            if (current->profile.txnCount <= 0)
            {
                current->profile.avgSpend = clearedAmount;
                current->profile.txnCount = 1;
            }
            else
            {
                double oldTotal = current->profile.avgSpend * current->profile.txnCount;
                current->profile.txnCount++;
                current->profile.avgSpend =
                    (oldTotal + clearedAmount) / current->profile.txnCount;
            }

            return true;
        }

        current = current->next;
    }

    return false;
}

bool AccountHashTable::updateLastActiveTimestamp(const string& accountID,
                                                  int timestamp)
{
    int index = hashFunc(accountID);
    Node* current = buckets[index];

    while (current != nullptr)
    {
        if (current->profile.accountID == accountID)
        {
            current->profile.lastActiveTimestamp = timestamp;
            return true;
        }

        current = current->next;
    }

    return false;
}

bool AccountHashTable::contains(const string& accountID) const
{
    AccountProfile found;
    return getProfile(accountID, found);
}

bool AccountHashTable::isEmpty() const
{
    return entryCount == 0;
}

int AccountHashTable::size() const
{
    return entryCount;
}

void AccountHashTable::displayTable() const
{
    if (isEmpty())
    {
        cout << "Account Hash Table is empty.\n";
        return;
    }

    cout << "\n========== ACCOUNT HASH TABLE ==========" << endl;

    for (int i = 0; i < tableCapacity; i++)
    {
        if (buckets[i] == nullptr)
        {
            continue;
        }

        cout << "Bucket " << i << ": ";
        const Node* current = buckets[i];

        while (current != nullptr)
        {
            cout << "[Account: " << current->profile.accountID
                 << ", Avg Spend: " << current->profile.avgSpend
                 << ", Home City: " << current->profile.homeCity
                 << ", Txn Count: " << current->profile.txnCount
                 << ", Last Active: " << current->profile.lastActiveTimestamp
                 << "]";

            if (current->next != nullptr)
            {
                cout << " -> ";
            }

            current = current->next;
        }

        cout << endl;
    }

    cout << "========================================\n";
}
