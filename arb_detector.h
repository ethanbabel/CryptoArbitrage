#ifndef ARB_DETECTOR_H
#define ARB_DETECTOR_H

#include <unordered_map>
#include <vector>
#include <limits>
#include <string>
#include <iostream>
#include "ses.h"

class ArbDetector {
    friend class ArbDetectorTest;  // Allow test class to access private members
public:
    ArbDetector();

    struct Edge {
        std::string from;
        std::string to;
        double weight;  // Log-transformed rate for Bellman-Ford
    };

    // Method to update the graph with new swap quote data
    void updateGraph(const std::string& from, const std::string& to, double price);

    // Runs Bellman-Ford to detect arbitrage cycles
    void detectArbitrage();

    // GETTER/SETTER METHODS FOR TESTING
    const std::unordered_map<std::string, std::vector<Edge>>& getGraph() const {
        return graph;
    }

    std::vector<std::string> runBellmanFord(const std::string& start) {
        return bellmanFord(start);
    }

    std::vector<std::string> testBellmanFord(const std::string& start) {
        return bellmanFord(start);
    }

    void setEmailer(SESEmailer& newEmailer) {
        emailer = newEmailer;
    }

private:

    std::unordered_map<std::string, std::vector<Edge>> graph;
    std::vector<std::string> tokens;

    SESEmailer emailer;

    // Helper function to log-transform prices
    double logTransform(double price);

    // Helper function to calculate arbitrage profit
    double calculateArbitrageProfit(const std::vector<std::string>& cycle);

    // Helper function to execute Bellman-Ford
    std::vector<std::string> bellmanFord(const std::string& start);
};

#endif // ARB_DETECTOR_H