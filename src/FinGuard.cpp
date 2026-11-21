#include "FinGuard.h"
#include <fstream>
#include <sstream>
#include <iomanip>

// PHASE 1.2: SHARED TRANSACTION SCHEMA DEFINITIONS
Transaction::Transaction() 
{
	transactionID = "";
	accountID = "";
	amount = 0.0;
	timestamp = 0;
	location = "";
	merchantType = "";
	riskScore = 0;
	flagReason = "NONE";
}

Transaction::Transaction(string id, string acc, double amt, int time, string loc, string merch) {
	transactionID = id;
	accountID = acc;
	amount = amt;
	timestamp = time;
	location = loc;
	merchantType = merch;
	riskScore = 0;
	flagReason = "NONE";
}

void Transaction::display() const 
{
	cout << left
		<< setw(9) << transactionID
		<< setw(9) << accountID
		<< "$" << setw(9) << fixed << setprecision(2) << amount
		<< setw(8) << timestamp
		<< setw(13) << location
		<< setw(14) << merchantType
		<< "Risk: " << setw(4) << riskScore
		<< "Flag: " << flagReason << endl;
}

// PHASE 1.3: CSV DATASET LOADER
int loadTransactionsFromCSV(const string& filename, Transaction* records, int maxCapacity)
{
	ifstream file(filename);
	if (!file.is_open()) 
	{
		cout << "Error: Could not open file " << filename << endl;
		return -1;
	}

	string line;
	int count = 0;

	// Skip the header line
	getline(file, line);

	while (getline(file, line) && count < maxCapacity) 
	{
		if (line.empty()) continue;

		stringstream ss(line);
		string id, acc, amtStr, timeStr, loc, merch;

		getline(ss, id, ',');
		getline(ss, acc, ',');
		getline(ss, amtStr, ',');
		getline(ss, timeStr, ',');
		getline(ss, loc, ',');
		getline(ss, merch, ',');

		if (!merch.empty() && merch.back() == '\r') 
		{
			merch.pop_back();
		}

		double amt = stod(amtStr);
		int time = stoi(timeStr);

		records[count] = Transaction(id, acc, amt, time, loc, merch);
		count++;
	}

	file.close();
	return count;
}

void displayTransactions(const Transaction* records, int count) {
	cout << "\n=========================================================================================\n";
	cout << "                        FINGUARD PHASE 1: LOADED TRANSACTION STREAM                      \n";
	cout << "=========================================================================================\n";
	cout << left
		<< setw(9) << "TXN_ID"
		<< setw(9) << "ACC_ID"
		<< setw(10) << "AMOUNT"
		<< setw(8) << "TIME"
		<< setw(13) << "LOCATION"
		<< setw(14) << "MERCHANT"
		<< setw(10) << "SCORE"
		<< "STATUS" << endl;
	cout << "-----------------------------------------------------------------------------------------\n";

	for (int i = 0; i < count; i++)
	{
		records[i].display();
	}

	cout << "=========================================================================================\n";
	cout << "Total Records Loaded: " << count << endl;
}