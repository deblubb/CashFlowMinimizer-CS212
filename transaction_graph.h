#ifndef TRANSACTION_GRAPH_H
#define TRANSACTION_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

// Stores transactions and calculates net balances.
class TransactionGraph {
public:

    // Add a transaction where one person owes another
    void addTransaction(string from, string to, double amount) {

        // Make sure both people exist
        ensureExists(from);
        ensureExists(to);

        // Save the transaction
        Edge e;
        e.from = from;
        e.to = to;
        e.amount = amount;
        edges_.push_back(e);

        // Update balances
        netBalance_[from] = netBalance_[from] - amount;
        netBalance_[to] = netBalance_[to] + amount;
    }

    // Split a payment evenly among a group
    void addSplitTransaction(string payer, vector<string> participants, double total) {

        double share = total / participants.size();

        for (int i = 0; i < (int)participants.size(); i++) {
            string p = participants[i];

            // Skip the payer
            if (p != payer) {
                addTransaction(p, payer, share);
            }
        }
    }

    // Number of recorded transactions
    int edgeCount() {
        return (int)edges_.size();
    }

    // Return net balances
    unordered_map<string, double>& getNetBalances() {
        return netBalance_;
    }

    // Print all transactions
    void printGraph() {
        cout << "=== Transaction Graph ===\n";
        for (int i = 0; i < (int)edges_.size(); i++) {
            Edge e = edges_[i];
            cout << "  " << e.from << " -> " << e.to
                 << "  $" << fixed << setprecision(2) << e.amount << "\n";
        }
        cout << "\n";
    }

    // Print each person's balance
    void printNetBalances() {
        cout << "=== Net Balances ===\n";

        for (auto person : netBalance_) {
            string name = person.first;
            double balance = person.second;

            cout << "  " << setw(12) << left << name << " : ";

            if (balance > 0.001) {
                cout << "+$" << fixed << setprecision(2) << balance
                     << "  (debtor - owes money)\n";
            }
            else if (balance < -0.001) {
                cout << " $" << fixed << setprecision(2) << balance
                     << "  (creditor - is owed money)\n";
            }
            else {
                cout << " $0.00  (settled)\n";
            }
        }
        cout << "\n";
    }

private:

    // One transaction
    struct Edge {
        string from;
        string to;
        double amount;
    };

    vector<Edge> edges_;
    unordered_map<string, double> netBalance_;

    // Add a person if they don't exist yet
    void ensureExists(string name) {
        if (netBalance_.find(name) == netBalance_.end()) {
            netBalance_[name] = 0.0;
        }
    }
};

#endif // TRANSACTION_GRAPH_H
