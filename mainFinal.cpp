#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "transaction_graph.h"
#include "greedy_settler.h"
#include "multiset_optimizer.h"

using namespace std;
// Parse a positive number
bool parsePositiveDouble(const std::string& token, double& out) {
    if (token.empty()) return false;
    try {
        size_t pos;
        double val = std::stod(token, &pos);

        // Reject invalid input
        if (pos != token.size()) return false;

        if (val <= 0.0) return false;
        out = val;
        return true;
    } catch (...) {
        return false;
    }
}

// Show available commands
void printHelp() {
    std::cout << "\nCommands:\n";
    std::cout << "  transaction  — record that one person owes another\n";
    std::cout << "  split        — split a bill equally among several people\n";
    std::cout << "  show         — print the current transaction graph and net balances\n";
    std::cout << "  done         — run the algorithms and show the settlement plan\n";
    std::cout << "  help         — show this menu\n\n";
}

// Input handlers

void handleTransaction(TransactionGraph& graph) {
    std::string fromRaw, toRaw, amtToken;
    double amount;

    std::cout << "  From (who owes): ";
    std::getline(std::cin, fromRaw);
    std::string from = normalizeName(fromRaw);
    if (from.empty()) {
        std::cout << "  Error: name cannot be empty.\n";
        return;
    }

    std::cout << "  To (who is owed): ";
    std::getline(std::cin, toRaw);
    std::string to = normalizeName(toRaw);
    if (to.empty()) {
        std::cout << "  Error: name cannot be empty.\n";
        return;
    }

    if (from == to) {
        std::cout << "  Error: a person cannot owe themselves.\n";
        return;
    }

    std::cout << "  Amount ($): ";
    std::getline(std::cin, amtToken);

    // Remove leading $
    if (!amtToken.empty() && amtToken[0] == '$')
        amtToken = amtToken.substr(1);

    if (!parsePositiveDouble(amtToken, amount)) {
        std::cout << "  Error: amount must be a positive number.\n";
        return;
    }

    graph.addTransaction(from, to, amount);
    std::cout << "  Recorded: " << from << " owes " << to
              << "  $" << std::fixed << std::setprecision(2) << amount << "\n";
}

void handleSplit(TransactionGraph& graph) {
    std::string payerRaw, amtToken, line;
    double total;

    std::cout << "  Who paid (payer): ";
    std::getline(std::cin, payerRaw);
    std::string payer = normalizeName(payerRaw);
    if (payer.empty()) {
        std::cout << "  Error: payer name cannot be empty.\n";
        return;
    }

    std::cout << "  Total bill amount ($): ";
    std::getline(std::cin, amtToken);
    if (!amtToken.empty() && amtToken[0] == '$')
        amtToken = amtToken.substr(1);

    if (!parsePositiveDouble(amtToken, total)) {
        std::cout << "  Error: amount must be a positive number.\n";
        return;
    }

    std::cout << "  Enter all participants (including payer), one per line.\n";
    std::cout << "  Type 'done' when finished:\n";

    std::vector<std::string> participants;
    bool payerIncluded = false;

    while (true) {
        std::cout << "    Participant: ";
        std::getline(std::cin, line);
        std::string name = normalizeName(line);

        if (name == "done") break;

        if (name.empty()) {
            std::cout << "    Error: name cannot be empty, try again.\n";
            continue;
        }

        // Skip duplicates
        bool duplicate = false;
        for (const auto& p : participants)
            if (p == name) { duplicate = true; break; }

        if (duplicate) {
            std::cout << "    '" << name << "' already added, skipping.\n";
            continue;
        }

        participants.push_back(name);
        if (name == payer) payerIncluded = true;
    }

    if (participants.size() < 2) {
        std::cout << "  Error: need at least 2 participants to split a bill.\n";
        return;
    }

    // Add payer if missing
    if (!payerIncluded) {
        std::cout << "  Note: payer '" << payer
                  << "' was not in the list — adding them automatically.\n";
        participants.push_back(payer);
    }

    double share = total / participants.size();
    std::cout << "  Split: $" << std::fixed << std::setprecision(2) << total
              << " among " << participants.size()
              << " people = $" << share << " each\n";

    graph.addSplitTransaction(payer, participants, total);

    std::cout << "  Recorded split bill (each non-payer owes "
              << payer << "  $" << share << ")\n";
}

// Main program
int main() {
    std::cout << "======================================\n";
    std::cout << "       Cash Flow Minimizer\n";
    std::cout << "======================================\n";
    printHelp();

    TransactionGraph graph;
    std::string input;

    while (true) {
        std::cout << "> ";

        if (!std::getline(std::cin, input)) break; // EOF

        std::string cmd = normalizeName(input);

        if (cmd == "Done") {
            break;

        } else if (cmd == "Transaction") {
            handleTransaction(graph);

        } else if (cmd == "Split") {
            handleSplit(graph);

        } else if (cmd == "Show") {
            graph.printGraph();
            graph.printNetBalances();

        } else if (cmd == "Help") {
            printHelp();

        } else if (cmd.empty()) {
            // Ignore empty input

        } else {
            std::cout << "  Unknown command '" << cmd
                      << "'. Type 'help' for options.\n";
        }
    }

    // Need at least one transaction
    if (graph.edgeCount() == 0) {
        std::cout << "\nNo transactions recorded. Exiting.\n";
        return 0;
    }

    std::cout << "\n======================================\n";
    std::cout << "         Running Algorithms\n";
    std::cout << "======================================\n\n";

    // Show current graph
    graph.printGraph();
    graph.printNetBalances();

    const auto& netBalances = graph.getNetBalances();

    // Greedy solution
    GreedySettler greedy;
    greedy.solve(netBalances);
    greedy.printSettlements();

    // Optimized solution
    MultisetOptimizer optimizer;
    optimizer.solve(netBalances);
    optimizer.printGroups();
    optimizer.printAllSettlements();

    // Compare results
    std::cout << "=== Efficiency Comparison ===\n";
    std::cout << "  Raw transactions entered : " << graph.edgeCount() << "\n";
    std::cout << "  Greedy settlement        : "
              << greedy.transactionCount() << " transactions\n";
    std::cout << "  Optimal settlement       : "
              << optimizer.transactionCount() << " transactions\n";

    int saved = greedy.transactionCount() - optimizer.transactionCount();
    if (saved > 0)
        std::cout << "  Transactions saved       : " << saved << "\n";
    else
        std::cout << "  No improvement (greedy already optimal for this input)\n";

    // Complexity summary
    std::cout << "\n=== Algorithm Complexity ===\n";
    std::cout << "  Graph construction    : O(E)        E = raw transactions\n";
    std::cout << "  Net balance pass      : O(E)\n";
    std::cout << "  Greedy settler        : O(n log n)  n = people\n";
    std::cout << "  Multiset optimizer    : O(2^n) worst case, O(n^2) best case\n\n";

    return 0;
}
