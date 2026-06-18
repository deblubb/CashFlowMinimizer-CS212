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
// Format a name consistently
string normalizeName(const string& raw) {
    string s = raw;

    // Remove extra whitespace
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    s = s.substr(start, end - start + 1);

    // Convert to lowercase
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return tolower(c);
    });

    // Capitalize first letter
    s[0] = static_cast<char>(toupper(static_cast<unsigned char>(s[0])));

    // Capitalize letters after spaces
    for (size_t i = 1; i < s.length(); ++i) {
        if (isspace(static_cast<unsigned char>(s[i - 1]))) {
            s[i] = static_cast<char>(toupper(static_cast<unsigned char>(s[i])));
        }
    }

    return s;
}

// Parse a positive number
bool parsePositiveDouble(const string& token, double& out) {
    if (token.empty()) return false;
    try {
        size_t pos;
        double val = stod(token, &pos);

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
    cout << "\nCommands:\n";
    cout << "  transaction  — record that one person owes another\n";
    cout << "  split        — split a bill equally among several people\n";
    cout << "  show         — print the current transaction graph and net balances\n";
    cout << "  done         — run the algorithms and show the settlement plan\n";
    cout << "  help         — show this menu\n\n";
}

// Input handlers

void handleTransaction(TransactionGraph& graph) {
    string fromRaw, toRaw, amtToken;
    double amount;

    cout << "  From (who owes): ";
    getline(cin, fromRaw);
    string from = normalizeName(fromRaw);
    if (from.empty()) {
        cout << "  Error: name cannot be empty.\n";
        return;
    }

    cout << "  To (who is owed): ";
    getline(cin, toRaw);
    string to = normalizeName(toRaw);
    if (to.empty()) {
        cout << "  Error: name cannot be empty.\n";
        return;
    }

    if (from == to) {
        cout << "  Error: a person cannot owe themselves.\n";
        return;
    }

    cout << "  Amount ($): ";
    getline(cin, amtToken);

    // Remove leading $
    if (!amtToken.empty() && amtToken[0] == '$')
        amtToken = amtToken.substr(1);

    if (!parsePositiveDouble(amtToken, amount)) {
        cout << "  Error: amount must be a positive number.\n";
        return;
    }

    graph.addTransaction(from, to, amount);
    cout << "  Recorded: " << from << " owes " << to
              << "  $" << fixed << setprecision(2) << amount << "\n";
}

void handleSplit(TransactionGraph& graph) {
    string payerRaw, amtToken, line;
    double total;

    cout << "  Who paid (payer): ";
    getline(cin, payerRaw);
    string payer = normalizeName(payerRaw);
    if (payer.empty()) {
        cout << "  Error: payer name cannot be empty.\n";
        return;
    }

    cout << "  Total bill amount ($): ";
    getline(cin, amtToken);
    if (!amtToken.empty() && amtToken[0] == '$')
        amtToken = amtToken.substr(1);

    if (!parsePositiveDouble(amtToken, total)) {
        cout << "  Error: amount must be a positive number.\n";
        return;
    }

    cout << "  Enter all participants (including payer), one per line.\n";
    cout << "  Type 'done' when finished:\n";

    vector<string> participants;
    bool payerIncluded = false;

    while (true) {
        cout << "    Participant: ";
        getline(cin, line);
        string name = normalizeName(line);

        if (name == "done") break;

        if (name.empty()) {
            cout << "    Error: name cannot be empty, try again.\n";
            continue;
        }

        // Skip duplicates
        bool duplicate = false;
        for (const auto& p : participants)
            if (p == name) { duplicate = true; break; }

        if (duplicate) {
            cout << "    '" << name << "' already added, skipping.\n";
            continue;
        }

        participants.push_back(name);
        if (name == payer) payerIncluded = true;
    }

    if (participants.size() < 2) {
        cout << "  Error: need at least 2 participants to split a bill.\n";
        return;
    }

    // Add payer if missing
    if (!payerIncluded) {
        cout << "  Note: payer '" << payer
                  << "' was not in the list — adding them automatically.\n";
        participants.push_back(payer);
    }

    double share = total / participants.size();
    cout << "  Split: $" << fixed << setprecision(2) << total
              << " among " << participants.size()
              << " people = $" << share << " each\n";

    graph.addSplitTransaction(payer, participants, total);

    cout << "  Recorded split bill (each non-payer owes "
              << payer << "  $" << share << ")\n";
}

// Main program
int main() {
    cout << "======================================\n";
    cout << "       Cash Flow Minimizer\n";
    cout << "======================================\n";
    printHelp();

    TransactionGraph graph;
    string input;

    while (true) {
        cout << "> ";

        if (!getline(cin, input)) break; // EOF

        string cmd = normalizeName(input);

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
            cout << "  Unknown command '" << cmd
                      << "'. Type 'help' for options.\n";
        }
    }

    // Need at least one transaction
    if (graph.edgeCount() == 0) {
        cout << "\nNo transactions recorded. Exiting.\n";
        return 0;
    }

    cout << "\n======================================\n";
    cout << "         Running Algorithms\n";
    cout << "======================================\n\n";

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
    cout << "=== Efficiency Comparison ===\n";
    cout << "  Raw transactions entered : " << graph.edgeCount() << "\n";
    cout << "  Greedy settlement        : "
              << greedy.transactionCount() << " transactions\n";
    cout << "  Optimal settlement       : "
              << optimizer.transactionCount() << " transactions\n";

    int saved = greedy.transactionCount() - optimizer.transactionCount();
    if (saved > 0)
        cout << "  Transactions saved       : " << saved << "\n";
    else
        cout << "  No improvement (greedy already optimal for this input)\n";

    // Complexity summary
    cout << "\n=== Algorithm Complexity ===\n";
    cout << "  Graph construction    : O(E)        E = raw transactions\n";
    cout << "  Net balance pass      : O(E)\n";
    cout << "  Greedy settler        : O(n log n)  n = people\n";
    cout << "  Multiset optimizer    : O(2^n) worst case, O(n^2) best case\n\n";

    return 0;
}
