#include "driver.h"
#include <iostream>
#include <chrono>
#include <thread>

// DRIVER IMPLEMENTATION

// Constructor initializes API key, prepares token pairs
Driver::Driver(const std::string& apiKey) : apiKey(apiKey), running(true) {
    fetchAllTokens();
    distributeTokenPairs();
    priceFetcher.setApiKey(apiKey);
}

// Destructor ensures threads are joined
Driver::~Driver() {
    running = false;

   if (fetcherThread.joinable()) {
        fetcherThread.join();
    }

    if (arbDetectorThread.joinable()) {
        arbDetectorThread.join();
    }
    std::cerr << "Driver shutdown complete." << std::endl << std::flush;
}

void Driver::fetchAllTokens() {
    std::cout << "Reading available tokens from file..." << std::endl;
    
    std::ifstream file("tokens.txt");
    if (!file) {
        std::cerr << "❌ ERROR: Could not open tokens.txt. Make sure it exists!" << std::endl;
        return;
    }

    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(file, token)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    file.close();

    if (tokens.empty()) {
        std::cerr << "❌ ERROR: No tokens found in tokens.txt." << std::endl;
        return;
    }

    // Generate all possible token pairs
    for (size_t i = 0; i < tokens.size(); i++) {
        for (size_t j = i + 1; j < tokens.size(); j++) {
            tokenPairs.emplace_back(tokens[i], tokens[j]);
        }
    }

    std::cout << "✅ Loaded " << tokens.size() << " tokens and created " << tokenPairs.size() << " token pairs." << std::endl;
}

// Assign token pairs to price fetcher
void Driver::distributeTokenPairs() {
    std::cout << "Assigning all token pairs to price fetcher..." << std::endl;
    for (const auto& pair : tokenPairs) {
        priceFetcher.addPair(pair.first, pair.second);
    }
}

// Runs Price Fetcher with Moving Window Rate Limiter
void Driver::priceFetcherThread() {
    std::cout << "Initializing price fetcher..." << std::endl;

    const int batchSize = 27;  // Number of requests per batch
    const int targetRate = 30; // Target API requests per second
    const int intervalMs = 1000 / (targetRate / batchSize); // Time per batch

    while (running) {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Fetch a batch of prices asynchronously
        std::vector<std::tuple<std::string, std::string, double>> newPrices = priceFetcher.fetchPricesAsyncBatch(batchSize);

        // Store new prices in the update queue
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            for (const auto& newPrice : newPrices) {
                if (std::get<2>(newPrice) > 0) {
                    priceUpdateQueue[{std::get<0>(newPrice), std::get<1>(newPrice)}] = std::get<2>(newPrice);
                }
            }
        }

        // Calculate elapsed time and adjust sleep duration dynamically
        auto endTime = std::chrono::high_resolution_clock::now();
        int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        int sleepTime = intervalMs - elapsedMs;
        
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
    }
}

// Runs the arbitrage detector in its own thread
void Driver::runArbDetector() {
    std::cout << "Initializing arbitrage detector..." << std::endl;
    while (running) {
        {
            // Apply all pending price updates before running Bellman-Ford
            std::lock_guard<std::mutex> lock(queueMutex);
            for (const auto& [pair, rate] : priceUpdateQueue) {
                arbDetector.updateGraph(pair.first, pair.second, rate);
            }
            priceUpdateQueue.clear();
        }

        // Run Bellman-Ford
        std::cout << "Running arbitrage detector with " << arbDetector.getNumTokens() << " total tokens and "<< arbDetector.getGraphNumEdges() << " total edges..." << std::endl;
        arbDetector.detectArbitrage();

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Run every 2 seconds (for now, adjust as needed)
    }
}

// Starts the driver, price fetchers, and arbitrage detector
void Driver::start() {
    std::cout << "Starting Driver..." << std::endl;

    // Start price fetchers in parallel threads
    fetcherThread = std::thread(&Driver::priceFetcherThread, this);

    // Start arbitrage detector in a separate thread
    arbDetectorThread = std::thread(&Driver::runArbDetector, this);
}