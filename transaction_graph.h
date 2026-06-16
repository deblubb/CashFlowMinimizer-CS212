#ifndef TRANSACTION_GRAPH_H
#define TRANSACTION_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <iomanip>

// ============================================================
//  TransactionGraph
//  Models a directed weighted graph of debts:
//    edge (u -> v, amount) means u owes v `amount` dollars.
//  After adding all edges, computeNetBalances() collapses the
//  graph into a single net value per person:
//    positive  => net creditor (others owe them)
//    negative  => net debtor   (they owe others)
//    zero      => balanced
// ============================================================
class TransactionGraph {
public:
    // Add a transaction: `from` owes `to` the given amount
    void addTransaction(const std::string& from,
                        const std::string& to,
                        double amount) {
        // Register both people
        ensureExists(from);
        ensureExists(to);

        // Store raw edge for display
        edges_.push_back({from, to, amount});

        // Accumulate net: debtor loses, creditor gains
        netBalance_[from] -= amount;
        netBalance_[to]   += amount;
    }

    // Compute and return the net balance map
    // (also cached internally for other modules to read)
    const std::unordered_map<std::string, double>& computeNetBalances() {
        return netBalance_;
    }

    // Print the raw transaction graph
    void printGraph() const {
        std::cout << "=== Transaction Graph ===\n";
        for (const auto& e : edges_) {
            std::cout << "  " << e.from
                      << " -> " << e.to
                      << "  $" << std::fixed << std::setprecision(2) << e.amount
                      << "\n";
        }
        std::cout << "\n";
    }

    // Print net balances
    void printNetBalances() const {
        std::cout << "=== Net Balances ===\n";
        for (const auto& [name, bal] : netBalance_) {
            std::cout << "  " << std::setw(12) << std::left << name << " : ";
            if (bal > 0.001)
                std::cout << "+$" << std::fixed << std::setprecision(2) << bal
                          << "  (creditor)\n";
            else if (bal < -0.001)
                std::cout << " $" << std::fixed << std::setprecision(2) << bal
                          << "  (debtor)\n";
            else
                std::cout << " $0.00  (settled)\n";
        }
        std::cout << "\n";
    }

    // Read-only access for other modules
    const std::unordered_map<std::string, double>& getNetBalances() const {
        return netBalance_;
    }

private:
    struct Edge {
        std::string from, to;
        double amount;
    };

    std::vector<Edge> edges_;
    std::unordered_map<std::string, double> netBalance_;

    void ensureExists(const std::string& name) {
        if (netBalance_.find(name) == netBalance_.end())
            netBalance_[name] = 0.0;
    }
};

#endif // TRANSACTION_GRAPH_H