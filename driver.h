#ifndef DRIVER_H
#define DRIVER_H

#include "arb_detector.h"
#include "price_fetcher.h"
#include <vector>
#include <unordered_map>
#include <memory>  // For unique_ptr
#include <thread>
#include <mutex>
#include <atomic>


// Custom hash function for pair keys
struct PairHash {
    template <typename T, typename U>
    std::size_t operator()(const std::pair<T, U>& p) const {
        return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
    }
};

class Driver {
public:
    explicit Driver(const std::string& apiKey);
    ~Driver();
    void start();

private:
    void fetchAllTokens();
    void distributeTokenPairs();
    void runPriceFetchers();
    void priceFetcherThread();
    void runArbDetector();

    std::string apiKey;
    std::vector<std::pair<std::string, std::string>> tokenPairs;
    PriceFetcher priceFetcher; 
    ArbDetector arbDetector;

    std::thread fetcherThread;
    std::thread arbDetectorThread;

    std::mutex queueMutex;
    std::unordered_map<std::pair<std::string, std::string>, double, PairHash> priceUpdateQueue;
    std::atomic<bool> running;
};

#endif // DRIVER_H