#include <iostream>
#include "transaction_graph.h"
#include "greedy_settler.h"
#include "multiset_optimizer.h"

// ============================================================
//  main.cpp  --  Cash Flow Minimizer 
//
//  Uses hardcoded test data for now.
//
//  Test scenario:
//    Alice owes Bob $30
//    Bob   owes Carol $20
//    Carol owes Dave $10
//    Dave  owes Alice $15
//    Eve   owes Frank $25
//    Frank owes Eve $25       
// ============================================================
int main() {
    std::cout << "======================================\n";
    std::cout << "       Cash Flow Minimizer\n";
    std::cout << "======================================\n\n";

    // STEP 1: Build the transaction graph
    TransactionGraph graph;

    // --- Hardcoded test transactions 
    graph.addTransaction("Alice", "Bob",   30.00);
    graph.addTransaction("Bob",   "Carol", 20.00);
    graph.addTransaction("Carol", "Dave",  10.00);
    graph.addTransaction("Dave",  "Alice", 15.00);
    graph.addTransaction("Eve",   "Frank", 25.00);  
    graph.addTransaction("Frank", "Eve",   25.00);  

    // Print raw graph
    graph.printGraph();

    // STEP 2: Compute net balances
    const auto& netBalances = graph.computeNetBalances();
    graph.printNetBalances();

    // STEP 3: Greedy algorithm (heap-based)
    GreedySettler greedy;
    greedy.solve(netBalances);
    greedy.printSettlements();

    // STEP 4: Multiset sub-group optimizer
    MultisetOptimizer optimizer;
    optimizer.solve(netBalances);
    optimizer.printGroups();
    optimizer.printAllSettlements();

    // STEP 5: Comparison (baseline vs. optimized)
    std::cout << "=== Efficiency Comparison ===\n";
    std::cout << "  Greedy (unoptimized) : "
              << greedy.transactionCount() << " transactions\n";
    std::cout << "  Multiset (optimized) : "
              << optimizer.transactionCount() << " transactions\n";

    int saved = greedy.transactionCount() - optimizer.transactionCount();
    if (saved > 0)
        std::cout << "  Transactions saved   : " << saved << "\n";
    else
        std::cout << "  No improvement (greedy already optimal for this input)\n";

    std::cout << "\n";

    // Algorithm Complexity Summary
    std::cout << "=== Algorithm Complexity ===\n";
    std::cout << "  Transaction graph construction : O(E)       E = # transactions\n";
    std::cout << "  Net balance computation        : O(E)       single pass\n";
    std::cout << "  Greedy settler (heap-based)    : O(n log n) n = # people\n";
    std::cout << "  Multiset sub-group search      : O(2^n)     worst case (NP-hard)\n";
    std::cout << "                                   O(n^2)     best case (many sub-groups)\n";
    std::cout << "\n";

    return 0;
}