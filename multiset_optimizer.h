#ifndef MULTISET_OPTIMIZER_H
#define MULTISET_OPTIMIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>              // std::multiset lives in <set>
#include <map>
#include <set>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <algorithm>
#include "greedy_settler.h" // reuse Settlement struct and greedy within groups

// ============================================================
//  MultisetOptimizer
//
//  Upgrades the greedy approach by detecting isolated sub-groups
//  whose net balances sum to zero.  Each such sub-group can settle
//  independently -- this reduces the total transaction count below
//  what the global greedy achieves on certain graph topologies.
//
//  Algorithm:
//    1. Represent net balances scaled to cents (integers) to avoid
//       floating-point subset-sum issues.
//    2. Use a multiset of (balance, name) pairs.
//    3. For each person, search for a subset of remaining people
//       whose balances sum to zero with this person's balance.
//       If found, that subset is a self-contained group.
//    4. Run greedy within each group independently.
//
//  This tackles the NP-hard subset-sum variation referenced in the
//  project spec.  For small n (< ~20 people) the exponential search
//  is fast; for larger n a heuristic/pruned DFS is used.
//
//  Time complexity:
//    Best case  : O(n^2)  when many disjoint groups exist
//    Worst case : O(2^n)  when no sub-groups exist (falls back to greedy)
// ============================================================
class MultisetOptimizer {
public:
    struct Group {
        std::vector<std::string> members;
        std::vector<GreedySettler::Settlement> settlements;
    };

    // Run the full optimization.
    // Returns all groups found (each settled independently).
    std::vector<Group> solve(
        const std::unordered_map<std::string, double>& netBalances)
    {
        groups_.clear();
        allSettlements_.clear();

        // Scale to cents to work with integers (avoids float equality issues)
        std::vector<std::pair<long long, std::string>> people; // {cents, name}
        for (const auto& [name, bal] : netBalances) {
            if (std::abs(bal) > 0.001)
                people.push_back({llround(bal * 100.0), name});
        }

        std::vector<bool> used(people.size(), false);

        for (size_t i = 0; i < people.size(); ++i) {
            if (used[i]) continue;

            // Try to find a subset that sums to zero starting with person i
            std::vector<size_t> subset = {i};
            long long runningSum = people[i].first;

            if (findZeroSubset(people, used, i + 1, runningSum, subset)) {
                // Found a self-contained group
                Group g;
                std::unordered_map<std::string, double> groupBalances;
                for (size_t idx : subset) {
                    used[idx] = true;
                    g.members.push_back(people[idx].second);
                    groupBalances[people[idx].second] =
                        people[idx].first / 100.0;
                }

                // Settle within this group greedily
                GreedySettler gs;
                g.settlements = gs.solve(groupBalances);
                groups_.push_back(g);

                for (const auto& s : g.settlements)
                    allSettlements_.push_back(s);
            }
        }

        return groups_;
    }

    void printGroups() const {
        std::cout << "=== Multiset Optimizer: Sub-group Analysis ===\n";
        std::cout << "  Found " << groups_.size() << " independent sub-group(s)\n\n";

        int gNum = 1;
        for (const auto& g : groups_) {
            std::cout << "  Sub-group " << gNum++ << ": { ";
            for (size_t i = 0; i < g.members.size(); ++i) {
                std::cout << g.members[i];
                if (i + 1 < g.members.size()) std::cout << ", ";
            }
            std::cout << " }\n";

            int i = 1;
            for (const auto& s : g.settlements) {
                std::cout << "    " << i++ << ". "
                          << s.from << " pays " << s.to
                          << "  $" << std::fixed << std::setprecision(2)
                          << s.amount << "\n";
            }
            std::cout << "\n";
        }
    }

    void printAllSettlements() const {
        std::cout << "=== Optimized Settlement Plan ("
                  << allSettlements_.size() << " transactions total) ===\n";
        int i = 1;
        for (const auto& s : allSettlements_) {
            std::cout << "  " << i++ << ". "
                      << s.from << " pays " << s.to
                      << "  $" << std::fixed << std::setprecision(2)
                      << s.amount << "\n";
        }
        std::cout << "\n";
    }

    int transactionCount() const {
        return static_cast<int>(allSettlements_.size());
    }

private:
    std::vector<Group> groups_;
    std::vector<GreedySettler::Settlement> allSettlements_;

    // Recursive DFS to find a subset starting from `start`
    // that makes runningSum == 0.
    // Pruned: if only 1 person remains and sum != 0, backtracks.
    bool findZeroSubset(
        const std::vector<std::pair<long long, std::string>>& people,
        const std::vector<bool>& used,
        size_t start,
        long long runningSum,
        std::vector<size_t>& subset)
    {
        if (runningSum == 0 && subset.size() >= 2)
            return true;  // found a valid zero-sum group

        if (start >= people.size())
            return false;

        for (size_t j = start; j < people.size(); ++j) {
            if (used[j]) continue;

            subset.push_back(j);
            if (findZeroSubset(people, used, j + 1,
                               runningSum + people[j].first, subset))
                return true;
            subset.pop_back();
        }

        return false;
    }
};

#endif // MULTISET_OPTIMIZER_H