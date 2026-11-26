# FinGuard – AI-Based Financial Fraud Monitoring System

A Data Structures Lab semester project built in C++ that simulates real-time financial transaction ingestion, evaluates suspicious spending patterns using heuristic fraud rules, and prioritizes high-risk fraud alerts for analyst review.

## Group Members
* **Hafiza Amna Akbar** (`25F-0078`) — `25f0078@cfd.nu.edu.pk`
* **Maham Sultan Butt** (`25F-0110`) — `25f0110@cfd.nu.edu.pk`

## Core Data Structures Implemented (From Scratch)
All five data structures are custom-built in C++ without using built-in STL containers:

| Data Structure | Role in FinGuard | Time Complexity |
| :--- | :--- | :--- |
| **Queue (FIFO)** | Buffers incoming simulated transactions in exact arrival order before processing. | Enqueue: $O(1)$<br>Dequeue: $O(1)$ |
| **Hash Table** | Stores user account profiles (average spend, registered city, last active timestamp) for rapid baseline verification. | Avg Search: $O(1)$<br>Worst Case: $O(n)$ |
| **Stack (LIFO)** | Maintains an analyst audit log of resolved fraud alerts and supports undo/rollback actions. | Push: $O(1)$<br>Pop: $O(1)$ |
| **Binary Search Tree (BST)** | Indexes processed transactions by amount for sorted reporting and fast amount-range queries. | Avg Search: $O(\log n)$<br>Worst Case: $O(n)$ |
| **Max-Heap (Priority Queue)** | Ranks flagged transactions by `riskScore` so the highest-risk fraud alert is always resolved first. | Insert: $O(\log n)$<br>ExtractMax: $O(\log n)$ |

## Fraud Detection Rules
Each incoming transaction is evaluated against historical account baselines:
1. **Amount Spike (`+40` risk):** Transaction exceeds $3\times$ the user's historical average spend.
2. **Location Mismatch (`+30` risk):** Transaction city differs from the user's registered home city.
3. **Rapid Velocity Burst (`+35` risk):** Multiple transactions occur within a 5-minute window.
4. **High-Risk Merchant (`+25` risk):** Large transfers involving `Crypto` or `Wire` categories.

Transactions with a cumulative `riskScore >= 50` are automatically flagged and pushed into the **Max-Heap** triage queue.

## Repository Structure
```text
finguard-fraud-engine/
├── .gitignore
├── README.md
├── src/
│   ├── AccountHashTable.h       # Hash table declarations for fast account lookup
│   ├── AccountHashTable.cpp     # Hash table implementation for account profiles
│   ├── AlertMaxHeap.h           # Priority queue (max-heap) declarations for high-risk alerts
│   ├── AlertMaxHeap.cpp         # Max-heap implementation for ranking fraud alerts
│   ├── AuditStack.h             # Stack declarations for audit trail and rollback history
│   ├── AuditStack.cpp           # Stack implementation for alert review history
│   ├── FinGuard.h               # Shared transaction schema & core engine declarations
│   ├── FinGuard.cpp             # Core utility & CSV ingestion implementations
│   ├── FraudRuleEngine.h        # Fraud detection rules & risk scoring declarations
│   ├── FraudRuleEngine.cpp      # Rule evaluation logic (spikes, impossible travel, bursts)
│   ├── TransactionBST.h         # Binary search tree declarations for indexed queries
│   ├── TransactionBST.cpp       # BST implementation for time/amount transaction lookups
│   ├── TransactionQueue.h       # FIFO queue declarations for incoming transaction stream
│   ├── TransactionQueue.cpp     # Queue implementation for real-time stream processing
│   └── main.cpp                 # Entry point, CSV loader & interactive console interface
├── data/
│   └── transactions.csv         # Simulated normal and fraudulent transaction dataset
└── docs/                        # Project poster and submission documentation
