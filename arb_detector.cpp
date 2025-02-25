#include "arb_detector.h"
#include <cmath>
#include <unordered_map>
#include <limits>
#include <vector>

ArbDetector::ArbDetector() {
    // The constructor initializes an empty graph.
}

// Function to log-transform prices to make Bellman-Ford work with multiplicative arbitrage
double ArbDetector::logTransform(double price) {
    return -std::log(price);
}

// Function to update the master graph with new swap quote data
void ArbDetector::updateGraph(const std::string& from, const std::string& to, double price) {
    double weight = logTransform(price);
    
    graph[from].push_back({from, to, weight});
    
    // Ensure tokens are stored for Bellman-Ford iteration
    if (std::find(tokens.begin(), tokens.end(), from) == tokens.end()) {
        tokens.push_back(from);
    }
    if (std::find(tokens.begin(), tokens.end(), to) == tokens.end()) {
        tokens.push_back(to);
    }
}

// Bellman-Ford implementation to detect arbitrage cycles
std::vector<std::string> ArbDetector::bellmanFord(const std::string& start) {
    std::unordered_map<std::string, double> distances;
    std::unordered_map<std::string, std::string> predecessors;
    
    for (const auto& token : tokens) {
        distances[token] = std::numeric_limits<double>::infinity();
    }
    
    distances[start] = 0.0;

    // Relax edges |V| - 1 times
    for (size_t i = 0; i < tokens.size() - 1; ++i) {
        for (const auto& [node, edges] : graph) {
            for (const auto& edge : edges) {
                if (distances[edge.from] + edge.weight < distances[edge.to]) {
                    distances[edge.to] = distances[edge.from] + edge.weight;
                    predecessors[edge.to] = edge.from;
                }
            }
        }
    }

    // Check for negative cycles (arbitrage opportunities)
    for (const auto& [node, edges] : graph) {
        for (const auto& edge : edges) {
            if (distances[edge.from] + edge.weight < distances[edge.to]) {
                std::vector<std::string> cycle;
                std::string cycleNode = edge.to;

                // Track back the cycle
                for (size_t i = 0; i < tokens.size(); ++i) {
                    cycleNode = predecessors[cycleNode];
                }

                std::string startNode = cycleNode;
                do {
                    cycle.push_back(cycleNode);
                    cycleNode = predecessors[cycleNode];
                } while (cycleNode != startNode);
                cycle.push_back(startNode);

                return cycle;
            }
        }
    }

    return {}; // No arbitrage cycle found
}

// Function to detect arbitrage and send an email if an opportunity exists
void ArbDetector::detectArbitrage() {
    for (const auto& token : tokens) {
        std::vector<std::string> cycle = bellmanFord(token);
        
        if (!cycle.empty()) {
            std::string message = "Arbitrage Opportunity Detected:\n";
            for (const auto& node : cycle) {
                message += node + " -> ";
            }
            message += cycle.front(); // Complete the cycle
            std::cout << message << std::endl;
            emailer.sendEmail("Arbitrage Alert", message, "babelethan@gmail.com");
            // return;  // Stop checking after finding the first arbitrage opportunity
        }
    }
}