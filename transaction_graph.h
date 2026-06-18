#ifndef TRANSACTION_GRAPH_H
#define TRANSACTION_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

// ============================================================
//  TransactionGraph
//
//  Think of this like a notebook where we write down every
//  "X owes Y this much money" entry, then add it all up to
//  find out each person's FINAL balance.
//
//  A positive balance  = this person is OWED money (creditor)
//  A negative balance  = this person OWES money (debtor)
//  A zero balance      = this person is all settled up
// ============================================================
class TransactionGraph {
public:

    // Call this once for every debt.
    // Example: addTransaction("Alice", "Bob", 30.00)
    //          means Alice owes Bob $30.
    void addTransaction(string from, string to, double amount) {

        // Make sure both people already exist in our balance list
        // (starting at $0 if we haven't seen them yet)
        ensureExists(from);
        ensureExists(to);

        // Keep a record of the raw transaction (just for printing later)
        Edge e;
        e.from = from;
        e.to = to;
        e.amount = amount;
        edges_.push_back(e);

        // Update the running balances:
        // The person who owes money loses points
        netBalance_[from] = netBalance_[from] - amount;

        // The person who is owed money gains points
        netBalance_[to] = netBalance_[to] + amount;
    }

    // For splitting a bill evenly among a group.
    // Example: addSplitTransaction("Alice", {"Alice","Bob","Carol"}, 30.00)
    //          means Alice paid $30 total, so Bob and Carol each
    //          owe Alice their $10 share. Alice doesn't owe herself.
    void addSplitTransaction(string payer, vector<string> participants, double total) {

        double share = total / participants.size();

        for (int i = 0; i < (int)participants.size(); i++) {
            string p = participants[i];

            // The payer doesn't owe themselves a share
            if (p != payer) {
                addTransaction(p, payer, share);
            }
        }
    }

    // Returns how many individual transactions have been recorded so far
    int edgeCount() {
        return (int)edges_.size();
    }

    // Same as above, but doesn't make a copy (slightly faster)
    unordered_map<string, double>& getNetBalances() {
        return netBalance_;
    }

    // Prints every single transaction we recorded
    void printGraph() {
        cout << "=== Transaction Graph ===\n";
        for (int i = 0; i < (int)edges_.size(); i++) {
            Edge e = edges_[i];
            cout << "  " << e.from << " -> " << e.to
                 << "  $" << fixed << setprecision(2) << e.amount << "\n";
        }
        cout << "\n";
    }

    // Prints the final balance for each person
    void printNetBalances() {
        cout << "=== Net Balances ===\n";

        for (auto person : netBalance_) {
            string name = person.first;
            double balance = person.second;

            cout << "  " << setw(12) << left << name << " : ";

            if (balance > 0.001) {
                cout << "+$" << fixed << setprecision(2) << balance
                     << "  (creditor - is owed money)\n";
            }
            else if (balance < -0.001) {
                cout << " $" << fixed << setprecision(2) << balance
                     << "  (debtor - owes money)\n";
            }
            else {
                cout << " $0.00  (settled)\n";
            }
        }
        cout << "\n";
    }

private:

    // One single transaction: "from" owes "to" this much money
    struct Edge {
        string from;
        string to;
        double amount;
    };

    vector<Edge> edges_;                    // every transaction we've seen
    unordered_map<string, double> netBalance_;  // name -> final balance

    // If this person hasn't shown up before, give them a starting
    // balance of $0 so we don't crash when we try to update them.
    void ensureExists(string name) {
        if (netBalance_.find(name) == netBalance_.end()) {
            netBalance_[name] = 0.0;
        }
    }
};

#endif // TRANSACTION_GRAPH_H