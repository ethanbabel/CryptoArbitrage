#ifndef DRIVER_H
#define DRIVER_H

#include "arb_detector.h"
#include "price_fetcher.h"
#include "api_throttler.h"
#include "utils.h"
#include <vector>
// #include <unordered_map>
#include <memory> 
#include <thread>
// #include <mutex>
#include <atomic>
#include <optional>
#include "edge.h"
#include "graph_analytics.h"
#ifdef U
#undef U
#endif
#include "concurrentqueue.h"  // from concurrentqueue


// Custom hash function for pair keys
struct PairHash {
    template <typename T, typename U>
    std::size_t operator()(const std::pair<T, U>& p) const {
        return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
    }
};

class Driver {
public:
    explicit Driver(const std::string& apiKey, bool runGraphAnalytics);
    ~Driver();
    void start();

private:
    void fetchAllTokens();
    void startPriceFetcherThreads();
    void priceFetcherThread(PriceFetcher* fetcher, const std::vector<std::pair<std::string, std::string>>& pairs);
    void runArbDetector();

    std::string apiKey;
    bool runGraphAnalytics;

    std::vector<std::pair<std::string, std::string>> tokenPairs;
    std::vector<std::unique_ptr<PriceFetcher>> priceFetchers;
    ArbDetector arbDetector;

    std::vector<std::thread> fetcherThreads;
    std::thread arbDetectorThread;

    APIThrottler throttler;

    // std::mutex queueMutex;
    // std::unordered_map<std::pair<std::string, std::string>, double, PairHash> priceUpdateQueue;
    moodycamel::ConcurrentQueue<std::tuple<std::string, std::string, double>> priceUpdateQueue;

    std::atomic<bool> running;
};

#endif // DRIVER_H