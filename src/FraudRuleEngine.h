#pragma once
#include "FinGuard.h"
#include "AccountHashTable.h"

// ============================================================================
// PHASE 3.1: HEURISTIC FRAUD RULE ENGINE
// Evaluates each incoming transaction against the account's historical baseline
// stored in the AccountHashTable and assigns a composite riskScore (0 - 100)
// along with human-readable flagReason tags for any triggered rules.
// ============================================================================
class FraudRuleEngine
{
public:
    // Evaluates Rules A, B, C, and D on a dequeued transaction:
    //   - Rule A (Amount Spike):       amount > 3.0 * userAvgSpend           (+40 pts)
    //   - Rule B (Location Anomaly):   location != userHomeCity              (+30 pts)
    //   - Rule C (High Velocity):      > 3 transactions within 5 minutes     (+35 pts)
    //   - Rule D (High-Risk Merchant): Wire/Crypto merchant & amount > 10000 (+25 pts)
    //
    // Modifies `txn.riskScore` and `txn.flagReason` in place, and updates the
    // account's recent timestamp window inside `accountTable`.
    static void evaluateTransaction(Transaction& txn, AccountHashTable& accountTable);
};