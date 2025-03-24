#ifndef GRAPH_ANALYTICS_H
#define GRAPH_ANALYTICS_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <set>
#include <algorithm>
#include <string>
#include <chrono>
#include <iomanip>
#include "edge.h"

class GraphAnalytics {
public:
    GraphAnalytics(const std::unordered_map<std::string, std::vector<Edge>>& otherGraph);
    void findCycles();

private:
    std::unordered_map<std::string, std::vector<Edge>> graph;
    std::set<std::vector<std::string>> uniqueCycles;
    std::unordered_map<int, int> cycleSizeCount;
    std::unordered_map<std::string, int> tokenCycleFrequency;

    void dfs(const std::string& start, const std::string& node, std::vector<std::string>& path, std::unordered_set<std::string>& inStack);
    void saveCycleStats();
    std::vector<std::string> normalizeCycle(const std::vector<std::string>& cycle);
    void printCycle(const std::vector<std::string>& cycle);
};

#endif  // GRAPH_ANALYTICS_H