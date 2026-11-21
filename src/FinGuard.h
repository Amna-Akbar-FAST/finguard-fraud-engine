#pragma once
#include <iostream>
#include <string>

using namespace std;

// PHASE 1.2: SHARED TRANSACTION SCHEMA
struct Transaction 
{
	string transactionID;
	string accountID;
	double amount;
	int timestamp;            // Minutes from start of day (e.g., 540 = 09:00 AM)
	string location;
	string merchantType;
	int riskScore;
	string flagReason;

	Transaction();
	Transaction(string id, string acc, double amt, int time, string loc, string merch);
	void display() const;
};

// PHASE 1.3: SYNTHETIC DATASET LOADER
int loadTransactionsFromCSV(const string& filename, Transaction* records, int maxCapacity);
void displayTransactions(const Transaction* records, int count);