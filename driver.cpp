#include "driver.h"
#include <iostream>
#include <chrono>
#include <thread>

// Constructor initializes API key, prepares token pairs
Driver::Driver(const std::string& apiKey, bool runGraphAnalytics) : apiKey(apiKey), runGraphAnalytics(runGraphAnalytics), priceUpdateQueue(100000), running(true) {
    fetchAllTokens();
}

// Destructor ensures threads are joined
Driver::~Driver() {
    running = false;

    std::optional<std::unordered_map<std::string, std::vector<Edge>>> graphCopy = std::nullopt;
    if (runGraphAnalytics) {
        graphCopy = arbDetector.getGraphCopy();
    }

    for (auto& thread : fetcherThreads) {
        if (thread.joinable()) thread.join();
    }

    if (arbDetectorThread.joinable()) {
        arbDetectorThread.join();
    }
    std::cout << "Driver shutdown complete." << std::endl;

    if (runGraphAnalytics && graphCopy.has_value()) {
        std::cout << "📊 Running graph analytics on " << graphCopy.value().size() << " tokens...\n";
        GraphAnalytics analytics(graphCopy.value());
        analytics.findCycles();
    } else {
        std::cout << "Graph analytics skipped as per command-line parameter." << std::endl;
    }
}

void Driver::fetchAllTokens() {
    // Load fee tiers and available token pairs
    const std::string feeTierPath = "token_fee_tiers.csv";
    std::ifstream feeFile(feeTierPath);
    if (!feeFile.is_open()) {
        throw std::runtime_error("❌ ERROR: Could not open token fee tiers file: " + feeTierPath);
    }

    std::string line, baseSymbol, quoteSymbol, feeTier;
    std::getline(feeFile, line);  // Skip header

    while (std::getline(feeFile, line)) {
        std::stringstream ss(line);
        if (std::getline(ss, baseSymbol, ',') && std::getline(ss, quoteSymbol, ',') && std::getline(ss, feeTier, ',')) {
            baseSymbol = trim(baseSymbol);
            quoteSymbol = trim(quoteSymbol);

            // Also store the token pair
            tokenPairs.emplace_back(baseSymbol, quoteSymbol);
        }
    }

    feeFile.close();
    std::cout << "✅ Loaded " << tokenPairs.size() << " token pairs from " << feeTierPath << std::endl;
}

// Spawn all fetcher threads
void Driver::startPriceFetcherThreads() {
    // unsigned int numThreads = std::thread::hardware_concurrency() - 1;  // Leave one core for the arb detector
    unsigned int numThreads = tokenPairs.size() / 20;
    if (numThreads == 0) numThreads = 1;  // Default fallback

    std::cout << "🚀 Starting " << numThreads << " price fetcher threads...\n";

    int pairsPerThread = std::ceil(static_cast<float>(tokenPairs.size()) / numThreads);

    for (unsigned int i = 0; i < numThreads; ++i) {
        int start = i * pairsPerThread;
        int end = std::min(static_cast<int>(tokenPairs.size()), start + pairsPerThread);
        std::vector<std::pair<std::string, std::string>> subset(tokenPairs.begin() + start, tokenPairs.begin() + end);

        auto fetcher = std::make_unique<PriceFetcher>();
        fetcher->setApiKey(apiKey);
        fetcher->setThrottler(&throttler);

        PriceFetcher* rawPtr = fetcher.get();  // Safe reference before moving
        priceFetchers.emplace_back(std::move(fetcher));
        fetcherThreads.emplace_back(&Driver::priceFetcherThread, this, rawPtr, subset);
    }
}

// Fetcher loop (each thread runs this)
void Driver::priceFetcherThread(PriceFetcher* fetcher, const std::vector<std::pair<std::string, std::string>>& pairs) {
    const int batchSize = pairs.size();
    const int sleepMs = 0;  // Sleep time between fetches

    while (running) {
        std::vector<std::tuple<std::string, std::string, double>> newPrices = fetcher->fetchPricesAsyncBatch(batchSize, pairs);

        // {
        //     std::lock_guard<std::mutex> lock(queueMutex);
        //     for (const auto& [base, quote, price] : newPrices) {
        //         if (price > 0) {
        //             priceUpdateQueue[{base, quote}] = price;
        //         }
        //     }
        // }
        for (const auto& [base, quote, price] : newPrices) {
            if (price > 0) {
                priceUpdateQueue.enqueue(std::make_tuple(base, quote, price));
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }
}

// Runs the arbitrage detector in its own thread
void Driver::runArbDetector() {
    std::cout << "Initializing arbitrage detector..." << std::endl;
    while (running) {
        // {
        //     // Apply all pending price updates before running Bellman-Ford
        //     std::lock_guard<std::mutex> lock(queueMutex);
        //     for (const auto& [pair, rate] : priceUpdateQueue) {
        //         arbDetector.updateGraph(pair.first, pair.second, rate);
        //     }
        //     priceUpdateQueue.clear();
        // }
        std::tuple<std::string, std::string, double> update;
        while (priceUpdateQueue.try_dequeue(update)) {
            arbDetector.updateGraph(std::get<0>(update), std::get<1>(update), std::get<2>(update));
        }

        // Run Bellman-Ford
        std::cout << "Running arbitrage detector with " << arbDetector.getNumTokens() << " total tokens and "<< arbDetector.getGraphNumEdges() << " total edges..." << std::endl;
        
        // Log the RPS in long-running loop
        throttler.logRPS(); 

        arbDetector.detectArbitrage();

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Run every 2 seconds (for now, adjust as needed)
    }
}

// Starts the driver, price fetchers, and arbitrage detector
void Driver::start() {
    std::cout << "Starting Driver..." << std::endl;

    // Start price fetchers in parallel threads
    startPriceFetcherThreads();

    // Start arbitrage detector in a separate thread
    arbDetectorThread = std::thread(&Driver::runArbDetector, this);
}