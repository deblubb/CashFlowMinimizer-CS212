#ifndef GREEDY_SETTLER_H
#define GREEDY_SETTLER_H

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <cmath>

// ============================================================
//  GreedySettler
//
//  Implements the classic greedy debt-minimization algorithm:
//    1. Separate people into creditors (net > 0) and debtors (net < 0)
//    2. Use a max-heap for creditors and a max-heap for |debt| debtors
//    3. Repeatedly match the largest debtor with the largest creditor
//       and settle as much as possible in one transaction
//
//  This minimizes the number of transactions in the average case
//  (though NOT guaranteed optimal for every topology -- the
//  multiset optimizer handles that).
//
//  Time complexity: O(n log n)  where n = number of people
// ============================================================
class GreedySettler {
public:
    struct Settlement {
        std::string from;   // who pays
        std::string to;     // who receives
        double amount;
    };

    // Run the greedy algorithm on the provided net balances.
    // Returns the list of settlements needed.
    std::vector<Settlement> solve(
        const std::unordered_map<std::string, double>& netBalances)
    {
        settlements_.clear();

        // Max-heap: {amount, name}
        // Creditors: positive balance
        // Debtors:   stored as positive magnitude for easy comparison
        using Entry = std::pair<double, std::string>;
        std::priority_queue<Entry> creditors, debtors;

        for (const auto& [name, bal] : netBalances) {
            if (bal > 0.001)
                creditors.push({bal, name});
            else if (bal < -0.001)
                debtors.push({-bal, name});   // store magnitude
        }

        while (!creditors.empty() && !debtors.empty()) {
            auto [credit, creditor] = creditors.top(); creditors.pop();
            auto [debt,   debtor]   = debtors.top();   debtors.pop();

            double settled = std::min(credit, debt);
            settlements_.push_back({debtor, creditor, settled});

            double remainCredit = credit - settled;
            double remainDebt   = debt   - settled;

            // Push back whoever still has a remaining balance
            if (remainCredit > 0.001) creditors.push({remainCredit, creditor});
            if (remainDebt   > 0.001) debtors.push({remainDebt,   debtor});
        }

        return settlements_;
    }

    // Print the settlement plan
    void printSettlements() const {
        std::cout << "=== Greedy Settlement Plan (" 
                  << settlements_.size() << " transactions) ===\n";
        int i = 1;
        for (const auto& s : settlements_) {
            std::cout << "  " << i++ << ". "
                      << s.from << " pays " << s.to
                      << "  $" << std::fixed << std::setprecision(2) << s.amount
                      << "\n";
        }
        std::cout << "\n";
    }

    int transactionCount() const {
        return static_cast<int>(settlements_.size());
    }

private:
    std::vector<Settlement> settlements_;
};

#endif // GREEDY_SETTLER_H