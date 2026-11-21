#include "FinGuard.h"

int main() 
{
	const int MAX_RECORDS = 100;
	Transaction* transactions = new Transaction[MAX_RECORDS];

	string filePath = "../data/transactions.csv";
	int loadedCount = loadTransactionsFromCSV(filePath, transactions, MAX_RECORDS);

	if (loadedCount <= 0) 
	{
		filePath = "data/transactions.csv";
		loadedCount = loadTransactionsFromCSV(filePath, transactions, MAX_RECORDS);
	}

	if (loadedCount > 0) 
	{
		displayTransactions(transactions, loadedCount);
	}
	else {
		cout << "[WARNING] No transactions loaded. Verify 'data/transactions.csv' path." << endl;
	}

	delete[] transactions;
	return 0;
}