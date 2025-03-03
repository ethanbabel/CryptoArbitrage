#include "arb_detector.h"
#include <cmath>
#include <unordered_map>
#include <limits>
#include <vector>
#include <fstream>  // For file handling
#include <ctime>    // For timestamping

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

    bool edgeExists = false;
    for (auto& edge : graph[from]) {
        if (edge.to == to) {
            edgeExists = true;
            edge.weight = weight;  // Update the existing edge
            break;
        }
    }
    if (!edgeExists) {
        numEdges++;
        graph[from].push_back({from, to, weight});
    }
    
    
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

// Function to calculate the profit percentage of an arbitrage cycle
double ArbDetector::calculateArbitrageProfit(const std::vector<std::string>& cycle) {
    if (cycle.empty()) return 0.0; // No arbitrage cycle found

    double product = 1.0;

    // Multiply all exchange rates along the cycle
    for (size_t i = 0; i < cycle.size() - 1; ++i) {
        std::string from = cycle[i];
        std::string to = cycle[i + 1];

        // Find the edge corresponding to this transition
        auto it = graph.find(from);
        if (it != graph.end()) {
            for (const auto& edge : it->second) {
                if (edge.to == to) {
                    product *= std::exp(-edge.weight);  // Reverse log transform
                    break;
                }
            }
        }
    }

    // Final transition back to start
    std::string last = cycle.back();
    std::string first = cycle.front();
    auto it = graph.find(last);
    if (it != graph.end()) {
        for (const auto& edge : it->second) {
            if (edge.to == first) {
                product *= std::exp(-edge.weight);
                break;
            }
        }
    }

    // Calculate profit percentage
    double profit = (product - 1.0) * 100.0;
    return profit;
}

// Function to detect arbitrage, send an email, and log it
void ArbDetector::detectArbitrage() {
    std::ofstream logFile("arbitrage_log.txt", std::ios::app);  // Open in append mode

    if (!logFile) {
        std::cerr << "⚠️ ERROR: Could not open arbitrage_log.txt for writing!" << std::endl;
        return;
    }

    for (const auto& token : tokens) {
        std::vector<std::string> cycle = bellmanFord(token);

        if (!cycle.empty()) {
            double profit = calculateArbitrageProfit(cycle);

            // Only log & send emails if the arbitrage is profitable
            if (profit > 0.0) {
                // Construct the arbitrage message
                std::string message = "💰 Arbitrage Opportunity Detected:\n";
                for (const auto& node : cycle) {
                    message += node + " -> ";
                }
                message += cycle.front();  // Complete the cycle

                message += "\n📈 Expected Profit: " + std::to_string(profit) + "%";

                // Print to console
                std::cout << message << std::endl;

                // Send an email notification
                emailer.sendEmail("Arbitrage Alert", message, "babelethan@gmail.com");

                // Log to file with timestamp
                std::time_t now = std::time(nullptr);
                logFile << "[" << std::ctime(&now) << "] " << message << "\n\n";
            }
        }
    }

    logFile.close();  // Ensure the log file is properly closed
}