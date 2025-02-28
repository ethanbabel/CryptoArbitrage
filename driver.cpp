#include "driver.h"
#include <iostream>
#include <chrono>
#include <thread>

// Constructor initializes API key and prepares token pairs
Driver::Driver(const std::string& apiKey) : apiKey(apiKey), running(true) {
    fetchAllTokens();
    distributeTokenPairs();
}

// Destructor ensures threads are joined
Driver::~Driver() {
    running = false;

    for (auto& t : fetcherThreads) {
        if (t.joinable()) t.join();
    }

    if (arbDetectorThread.joinable()) {
        arbDetectorThread.join();
    }
    std::cerr << "Driver shutdown complete." << std::endl << std::flush;
}

// Fetch all tokens (Placeholder for now)
void Driver::fetchAllTokens() {
    std::cout << "Fetching all tokens from Ethereum chain..." << std::endl;
    
    std::vector<std::string> tokens = {"ETH", "USDT", "DAI", "WBTC", "LINK"};

    for (size_t i = 0; i < tokens.size(); i++) {
        for (size_t j = i + 1; j < tokens.size(); j++) {
            tokenPairs.emplace_back(tokens[i], tokens[j]);
        }
    }
    std::cout << "Fetched " << tokenPairs.size() << " token pairs." << std::endl;
}

// Distribute token pairs among price fetchers
void Driver::distributeTokenPairs() {
    std::cout << "Distributing token pairs among price fetchers..." << std::endl;

    size_t numFetchers = std::thread::hardware_concurrency();
    std::cout << "Detected " << numFetchers << " CPU cores. " << std::endl;
    priceFetchers.reserve(numFetchers);

    for (size_t i = 0; i < numFetchers; ++i) {
        priceFetchers.emplace_back(std::make_unique<PriceFetcher>());
        priceFetchers.back()->setApiKey(apiKey);  // Set API key
    }
    std::cout << "Created " << numFetchers << " price fetchers." << std::endl;

    for (size_t i = 0; i < tokenPairs.size(); i++) {
        priceFetchers[i % numFetchers]->addPair(tokenPairs[i].first, tokenPairs[i].second);
    }
    std::cout << "Distributed " << tokenPairs.size() << " token pairs." << std::endl;
}

// Runs all price fetchers in multiple threads
void Driver::runPriceFetchers() {
    std::cout << "Starting price fetcher threads..." << std::endl;

    for (auto& fetcher : priceFetchers) {
        fetcherThreads.emplace_back(&Driver::priceFetcherThread, this, std::ref(fetcher));
    }
}

// Price fetcher thread function (fetches prices and adds them to queue)
void Driver::priceFetcherThread(std::unique_ptr<PriceFetcher>& fetcher) {
    while (running) {
        auto newPrices = fetcher->fetchPrices();
        std::cout << "Fetched " << newPrices.size() << " new prices..." << std::endl;
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            for (const auto& [tokenA, tokenB, rate] : newPrices) {
                priceUpdateQueue[{tokenA, tokenB}] = rate; // Only store latest price
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // API rate limit handling
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
        std::cout << "Running arbitrage detector..." << std::endl;
        arbDetector.detectArbitrage();

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Run every 2 seconds (for now, adjust as needed)
    }
}

// Starts the driver, price fetchers, and arbitrage detector
void Driver::start() {
    std::cout << "Starting Driver..." << std::endl;

    // Start price fetchers in parallel threads
    runPriceFetchers();

    // Start arbitrage detector in a separate thread
    arbDetectorThread = std::thread(&Driver::runArbDetector, this);
}