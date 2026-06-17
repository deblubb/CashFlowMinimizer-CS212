#ifndef GREEDY_SETTLER_H
#define GREEDY_SETTLER_H

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

// ============================================================
//  GreedySettler
//
//  THE BIG IDEA:
//  Always match whoever owes the MOST money with whoever is
//  owed the MOST money. Pay off as much as possible, then
//  repeat until everyone is settled.
//
//  This wipes out the biggest debts first, so we end up using
//  way fewer transactions than if we matched people randomly.
//
//  Speed: O(n log n)   (n = number of people)
// ============================================================
class GreedySettler {
public:

    // One result: who pays, who gets paid, how much
    struct Settlement {
        string from;
        string to;
        double amount;
    };

    // Small helper struct so our heap can store BOTH
    // a dollar amount AND the person's name together.
    // (this replaces using a "pair" so it's easier to read)
    struct PersonAmount {
        double amount;
        string name;

        // This tells the heap HOW to compare two PersonAmounts.
        // We want the heap to always put the BIGGEST amount on top.
        bool operator<(const PersonAmount& other) const {
            return amount < other.amount;
        }
    };

    // Runs the greedy algorithm and returns the list of payments needed.
    vector<Settlement> solve(unordered_map<string, double> netBalances) {

        settlements_.clear();

        // A "max heap" (priority_queue) is a container that always
        // keeps the BIGGEST value on top, so grabbing the max is instant.
        priority_queue<PersonAmount> creditors;  // people who are OWED money
        priority_queue<PersonAmount> debtors;    // people who OWE money

        // Sort everyone into the two heaps based on their balance
        for (auto person : netBalances) {
            string name = person.first;
            double balance = person.second;

            if (balance > 0.001) {
                // Positive balance -> they're a creditor
                PersonAmount p;
                p.amount = balance;
                p.name = name;
                creditors.push(p);
            }
            else if (balance < -0.001) {
                // Negative balance -> they're a debtor.
                // We store the POSITIVE version of their debt
                // so the heap math works the same as for creditors.
                PersonAmount p;
                p.amount = -balance;
                p.name = name;
                debtors.push(p);
            }
        }

        // Keep matching the biggest debtor with the biggest creditor
        // until one of the heaps runs out of people.
        while (!creditors.empty() && !debtors.empty()) {

            PersonAmount topCreditor = creditors.top();
            creditors.pop();

            PersonAmount topDebtor = debtors.top();
            debtors.pop();

            // You can only pay off the SMALLER of the two amounts
            // (you can't pay more than you owe, or receive more than you're owed)
            double settledAmount = min(topCreditor.amount, topDebtor.amount);

            Settlement s;
            s.from = topDebtor.name;
            s.to = topCreditor.name;
            s.amount = settledAmount;
            settlements_.push_back(s);

            // Whatever is left over still needs to be settled,
            // so push it back onto the heap.
            double creditorLeftover = topCreditor.amount - settledAmount;
            double debtorLeftover = topDebtor.amount - settledAmount;

            if (creditorLeftover > 0.001) {
                PersonAmount p;
                p.amount = creditorLeftover;
                p.name = topCreditor.name;
                creditors.push(p);
            }

            if (debtorLeftover > 0.001) {
                PersonAmount p;
                p.amount = debtorLeftover;
                p.name = topDebtor.name;
                debtors.push(p);
            }
        }

        return settlements_;
    }

    // Prints every payment in the settlement plan
    void printSettlements() {
        cout << "=== Greedy Settlement Plan ("
             << settlements_.size() << " transactions) ===\n";

        for (int i = 0; i < (int)settlements_.size(); i++) {
            Settlement s = settlements_[i];
            cout << "  " << (i + 1) << ". " << s.from << " pays " << s.to
                 << "  $" << fixed << setprecision(2) << s.amount << "\n";
        }
        cout << "\n";
    }

    int transactionCount() {
        return (int)settlements_.size();
    }

private:
    vector<Settlement> settlements_;
};

#endif // GREEDY_SETTLER_H