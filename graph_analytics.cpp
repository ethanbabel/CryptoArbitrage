#include "graph_analytics.h"

GraphAnalytics::GraphAnalytics(const std::unordered_map<std::string, std::vector<Edge>>& otherGraph) {
    graph = otherGraph;
    std::cout << "📊 Graph analytics initialized with " << graph.size() << " nodes.\n";
}

void GraphAnalytics::findCycles() {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::unordered_set<std::string> inStack;
    std::vector<std::string> path;

    for (const auto& [node, _] : graph) {
        path.clear();
        inStack.clear();
        dfs(node, node, path, inStack);
    }

    for (const auto& cycle : uniqueCycles) {
        cycleSizeCount[cycle.size()]++;
        for (const auto& token : cycle) {
            tokenCycleFrequency[token]++;
        }
    }

    saveCycleStats();
    std::cout << "📊 Cycle stats saved to CSVs.\n";

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    int seconds = duration % 60;

    std::cout << "✅ findCycles() completed in ";
    std::cout << std::setfill('0') << std::setw(2) << hours << ":"
              << std::setw(2) << minutes << ":"
              << std::setw(2) << seconds << std::endl;
}

void GraphAnalytics::dfs(const std::string& start, const std::string& node, std::vector<std::string>& path, std::unordered_set<std::string>& inStack) {
    if (inStack.count(node)) {
        auto it = std::find(path.begin(), path.end(), node);
        if (it != path.end()) {
            std::vector<std::string> cycle(it, path.end());
            if (cycle.size() >= 3 && cycle.front() == node) {
                std::cout << "🔄 Found cycle: ";
                printCycle(cycle);
                std::vector<std::string> normCycle = normalizeCycle(cycle);
                uniqueCycles.insert(normCycle);
            }
        }
        return;
    }

    path.push_back(node);
    inStack.insert(node);

    for (const Edge& edge : graph[node]) {
        dfs(start, edge.to, path, inStack);
    }

    path.pop_back();
    inStack.erase(node);
}

std::vector<std::string> GraphAnalytics::normalizeCycle(const std::vector<std::string>& cycle) {
    std::vector<std::string> rotated = cycle;
    auto minIt = std::min_element(rotated.begin(), rotated.end());
    std::rotate(rotated.begin(), minIt, rotated.end());
    return rotated;
}

void GraphAnalytics::saveCycleStats() {
    // Save cycle size count
    std::ofstream sizeFile("cycle_size_count.csv");
    sizeFile << "CycleSize,CycleCount\n";
    for (const auto& [size, count] : cycleSizeCount) {
        sizeFile << size << "," << count << "\n";
    }
    sizeFile.close();

    // Save token frequency
    std::ofstream tokenFile("token_cycle_frequency.csv");
    tokenFile << "Token,Frequency\n";
    for (const auto& [token, freq] : tokenCycleFrequency) {
        tokenFile << token << "," << freq << "\n";
    }
    tokenFile.close();
}

void GraphAnalytics::printCycle(const std::vector<std::string>& cycle) {
    for (const auto& token : cycle) {
        std::cout << token << " -> ";
    }
    std::cout << cycle.front() << "\n";
}