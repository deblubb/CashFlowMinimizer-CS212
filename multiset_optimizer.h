#ifndef MULTISET_OPTIMIZER_H
#define MULTISET_OPTIMIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "greedy_settler.h"

using namespace std;

// ============================================================
//  MultisetOptimizer
//
//  THE BIG IDEA:
//  Sometimes a small group of people happen to cancel each
//  other out perfectly. Example: Eve is owed $25, and Frank
//  owes $25. They can settle with EACH OTHER and never need
//  to be involved with anyone else.
//
//  This class searches for these self-contained groups so we
//  can settle them separately, which can reduce the total
//  number of transactions needed overall.
//
//  This is a version of the "Subset Sum" problem, which is
//  NP-hard -- there's no known fast way to solve it for every
//  possible input, so we use a smart trial-and-error search
//  (called "backtracking") instead.
//
//  Speed: best case  O(n^2)    many groups found quickly
//         worst case O(2^n)    no groups exist, full search needed
// ============================================================
class MultisetOptimizer {
public:

    // A group of people who can settle independently from everyone else
    struct Group {
        vector<string> members;
        vector<GreedySettler::Settlement> settlements;
    };

    // Helper struct: one person's balance, stored as whole cents
    // (instead of dollars) so we can compare amounts exactly.
    // Computers can't reliably tell if two DECIMAL numbers are
    // exactly equal, but they CAN do this perfectly with whole numbers.
    struct PersonCents {
        long long cents;
        string name;
    };

    // Runs the search and returns every independent group found.
    vector<Group> solve(unordered_map<string, double> netBalances) {

        groups_.clear();
        allSettlements_.clear();

        // STEP 1: Convert every balance from dollars into whole cents.
        // Example: $10.50 becomes 1050 (no decimals, no rounding issues)
        vector<PersonCents> people;

        for (auto person : netBalances) {
            string name = person.first;
            double balance = person.second;

            if (abs(balance) > 0.001) {
                PersonCents pc;
                pc.cents = llround(balance * 100.0);
                pc.name = name;
                people.push_back(pc);
            }
        }

        // Keeps track of who we've already placed into a group
        vector<bool> used(people.size(), false);

        // STEP 2: For every person not yet grouped, try to find
        // a set of people (including them) whose balances add up to zero.
        for (int i = 0; i < (int)people.size(); i++) {
            if (used[i]) continue;

            vector<int> subset;
            subset.push_back(i);
            long long runningTotal = people[i].cents;

            bool found = findZeroSumGroup(people, used, i + 1, runningTotal, subset);

            if (found) {
                // We found a group of people whose balances cancel out!
                Group g;
                unordered_map<string, double> groupBalances;

                for (int j = 0; j < (int)subset.size(); j++) {
                    int idx = subset[j];
                    used[idx] = true;
                    g.members.push_back(people[idx].name);

                    // Convert back from cents to dollars
                    groupBalances[people[idx].name] = people[idx].cents / 100.0;
                }

                // Settle this small group using the greedy algorithm
                GreedySettler gs;
                g.settlements = gs.solve(groupBalances);
                groups_.push_back(g);

                for (int k = 0; k < (int)g.settlements.size(); k++) {
                    allSettlements_.push_back(g.settlements[k]);
                }
            }
        }

        return groups_;
    }

    // Prints each group we found, and how it was settled internally
    void printGroups() {
        cout << "=== Multiset Optimizer: Sub-group Analysis ===\n";
        cout << "  Found " << groups_.size() << " independent sub-group(s)\n\n";

        for (int g = 0; g < (int)groups_.size(); g++) {
            Group group = groups_[g];

            cout << "  Sub-group " << (g + 1) << ": { ";
            for (int i = 0; i < (int)group.members.size(); i++) {
                cout << group.members[i];
                if (i + 1 < (int)group.members.size()) cout << ", ";
            }
            cout << " }\n";

            for (int i = 0; i < (int)group.settlements.size(); i++) {
                GreedySettler::Settlement s = group.settlements[i];
                cout << "    " << (i + 1) << ". " << s.from << " pays " << s.to
                     << "  $" << fixed << setprecision(2) << s.amount << "\n";
            }
            cout << "\n";
        }
    }

    // Prints the combined settlement plan across all groups
    void printAllSettlements() {
        cout << "=== Optimized Settlement Plan ("
             << allSettlements_.size() << " transactions total) ===\n";

        for (int i = 0; i < (int)allSettlements_.size(); i++) {
            GreedySettler::Settlement s = allSettlements_[i];
            cout << "  " << (i + 1) << ". " << s.from << " pays " << s.to
                 << "  $" << fixed << setprecision(2) << s.amount << "\n";
        }
        cout << "\n";
    }

    int transactionCount() {
        return (int)allSettlements_.size();
    }

private:
    vector<Group> groups_;
    vector<GreedySettler::Settlement> allSettlements_;

    // ------------------------------------------------------
    // This is the "backtracking" search.
    //
    // The idea: try adding one more person to our subset.
    //   - If the running total hits exactly zero, we're done!
    //   - If not, try adding the NEXT person and recurse.
    //   - If that doesn't work out, REMOVE them (this is the
    //     "backtrack" step) and try a different person instead.
    //
    // It's like exploring a maze: walk forward, hit a dead end,
    // walk back to the last fork, and try a different path.
    // ------------------------------------------------------
    bool findZeroSumGroup(vector<PersonCents>& people,
                          vector<bool>& used,
                          int start,
                          long long runningTotal,
                          vector<int>& subset) {

        // Success! We found a group of 2 or more people that sums to zero.
        if (runningTotal == 0 && subset.size() >= 2) {
            return true;
        }

        // We've run out of people to try -- no group found this way.
        if (start >= (int)people.size()) {
            return false;
        }

        // Try adding each remaining person one at a time
        for (int j = start; j < (int)people.size(); j++) {
            if (used[j]) continue;  // skip people already in another group

            // Try including person j
            subset.push_back(j);

            bool success = findZeroSumGroup(
                people, used, j + 1, runningTotal + people[j].cents, subset
            );

            if (success) {
                return true;  // we found a working group, stop searching
            }

            // Didn't work out -- undo adding person j and try the next one
            subset.pop_back();
        }

        // None of the remaining people led to a zero-sum group
        return false;
    }
};

#endif // MULTISET_OPTIMIZER_H