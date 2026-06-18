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

// Finds groups whose balances already add up to zero
// so they can be settled separately.
class MultisetOptimizer {
public:

    // One independent settlement group
    struct Group {
        vector<string> members;
        vector<GreedySettler::Settlement> settlements;
    };

    // Stores a balance in cents
    struct PersonCents {
        long long cents;
        string name;
    };

    // Find all independent groups
    vector<Group> solve(unordered_map<string, double> netBalances) {

        groups_.clear();
        allSettlements_.clear();

        // Convert balances to cents
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

        // Track who is already in a group
        vector<bool> used(people.size(), false);

        // Try finding a zero-sum group for each person
        for (int i = 0; i < (int)people.size(); i++) {
            if (used[i]) continue;

            vector<int> subset;
            long long startTotal = people[i].cents;

            bool found = findZeroSumGroup(people, used, i, startTotal, subset);

            if (found) {
                Group g;
                unordered_map<string, double> groupBalances;

                for (int j = 0; j < (int)subset.size(); j++) {
                    int idx = subset[j];
                    used[idx] = true;
                    g.members.push_back(people[idx].name);

                    // Convert back to dollars
                    groupBalances[people[idx].name] = people[idx].cents / 100.0;
                }

                // Settle the group
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

    // Print each group and its transactions
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

    // Print all transactions together
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

    // Finds the SMALLEST possible zero-sum group starting with person
    // "start". Tries group size 2 first, then 3, then 4, and so on,
    // so it never accidentally swallows everyone into one giant group
    // when smaller independent groups actually exist.
    //
    // (The old version just took whichever zero-sum group it found
    // first via plain depth-first search, which depended on the
    // iteration order of unordered_map and could wrongly merge
    // separate groups into one.)
    bool findZeroSumGroup(vector<PersonCents>& people,
                          vector<bool>& used,
                          int startPerson,
                          long long startTotal,
                          vector<int>& subset) {

        int n = (int)people.size();

        // How many additional people are actually available to try
        int available = 0;
        for (int k = 0; k < n; k++) {
            if (!used[k] && k != startPerson) available++;
        }

        // Try every possible group size, smallest first.
        // targetExtra = how many MORE people to add on top of startPerson
        for (int targetExtra = 1; targetExtra <= available; targetExtra++) {
            vector<int> trial;
            trial.push_back(startPerson);

            if (searchExactSize(people, used, startPerson + 1,
                                startTotal, targetExtra, trial)) {
                subset = trial;
                return true;
            }
        }

        return false;
    }

    // Searches for a zero-sum group of EXACTLY startPerson + extraNeeded
    // more people, using backtracking. Only returns true if a group of
    // this exact size sums to zero.
    bool searchExactSize(vector<PersonCents>& people,
                         vector<bool>& used,
                         int start,
                         long long runningTotal,
                         int extraNeeded,
                         vector<int>& subset) {

        // Used up our budget of extra people: check if we hit zero
        if (extraNeeded == 0) {
            return runningTotal == 0;
        }

        // Not enough people left to reach the target size
        if (start >= (int)people.size()) {
            return false;
        }

        for (int j = start; j < (int)people.size(); j++) {
            if (used[j]) continue;

            subset.push_back(j);

            bool success = searchExactSize(
                people, used, j + 1, runningTotal + people[j].cents,
                extraNeeded - 1, subset
            );

            if (success) {
                return true;
            }

            subset.pop_back();
        }

        return false;
    }
};

#endif // MULTISET_OPTIMIZER_H