#include "FraudRuleEngine.h"

void FraudRuleEngine::evaluateTransaction(Transaction& txn, AccountHashTable& accountTable)
{
    int score = 0;          // Cumulative risk score across all triggered rules
    string reasons = "";    // Stores pipe-separated labels of all triggered rules

    // Helper lambda to cleanly join multiple triggered rule names (e.g., "RULE_A | RULE_B")
    auto appendReason = [&](const string& ruleTag)
        {
            if (!reasons.empty())
            {
                reasons += " | ";
            }
            reasons += ruleTag;
        };

    // Step 1: Retrieve the user's baseline profile from the Account Hash Table (O(1) average lookup)
    AccountProfile profile;
    bool hasProfile = accountTable.getProfile(txn.accountID, profile);

    if (hasProfile)
    {
        // --------------------------------------------------------------------
        // RULE A: AMOUNT SPIKE (+40 points)
        // Triggers if the transaction amount is more than 3x the user's
        // established historical average spend (requires at least 1 prior txn).
        // --------------------------------------------------------------------
        if (profile.txnCount > 0 && profile.avgSpend > 0.0 &&
            txn.amount > (3.0 * profile.avgSpend))
        {
            score += 40;
            appendReason("RULE_A_AMOUNT_SPIKE");
        }

        // --------------------------------------------------------------------
        // RULE B: LOCATION ANOMALY (+30 points)
        // Triggers if the transaction occurs in a city different from the
        // account's registered/initial home city (potential impossible travel).
        // --------------------------------------------------------------------
        if (!profile.homeCity.empty() && txn.location != profile.homeCity)
        {
            score += 30;
            appendReason("RULE_B_LOCATION_ANOMALY");
        }

        // --------------------------------------------------------------------
        // RULE C: HIGH VELOCITY (+35 points)
        // First, record the current transaction's timestamp into the account's
        // sliding window. Then count how many transactions occurred within the
        // past 5 minutes (inclusive). Triggers if strictly more than 3 (> 3).
        // --------------------------------------------------------------------
        accountTable.recordTransactionTimestamp(txn.accountID, txn.timestamp);

        int txnsIn5Min = accountTable.countTransactionsInWindow(txn.accountID, txn.timestamp, 5);
        if (txnsIn5Min > 3)
        {
            score += 35;
            appendReason("RULE_C_HIGH_VELOCITY");
        }
    }

    // ------------------------------------------------------------------------
    // RULE D: HIGH-RISK MERCHANT (+25 points)
    // Triggers if the merchant category is high-risk (Wire or Crypto) AND the
    // transaction amount exceeds $10,000. Evaluated even if no profile exists.
    // ------------------------------------------------------------------------
    bool isHighRiskMerchant = (txn.merchantType == "Wire" ||
        txn.merchantType == "Crypto" ||
        txn.merchantType == "Wire/Crypto");

    if (isHighRiskMerchant && txn.amount > 10000.0)
    {
        score += 25;
        appendReason("RULE_D_HIGH_RISK_MERCHANT");
    }

    // ------------------------------------------------------------------------
    // FINALIZE SCORE & STATUS
    // Cap the maximum riskScore at 100 so scores stay within the [0, 100] scale.
    // If no rules were triggered, mark the transaction as "CLEARED".
    // ------------------------------------------------------------------------
    if (score > 100)
    {
        score = 100;
    }

    txn.riskScore = score;
    txn.flagReason = reasons.empty() ? "CLEARED" : reasons;
}