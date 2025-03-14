#ifndef PRICE_FETCHER_H
#define PRICE_FETCHER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <cpprest/http_client.h>  // C++ REST SDK (Casablanca)
#include <cpprest/json.h>
#include <atomic>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace std;

class PriceFetcher {
private:
    string API_KEY;  // API Key is now set after construction
    const string BASE_URL = "https://rest.cryptoapis.io/v2/market-data/exchange-rates/by-symbol/";
    vector<pair<string, string>> tokenPairs;  // Store assigned token pairs
    const int API_RATE_LIMIT = 30;  // API rate limit (requests per second)
    size_t currentPairIndex = 0;

public:
    PriceFetcher() = default;  

    void setApiKey(const string& apiKey);
    void addPair(const string& base, const string& quote);

    std::vector<std::tuple<std::string, std::string, double>> fetchPricesAsyncBatch(int batchSize);
    pplx::task<std::tuple<std::string, std::string, double>> get_swap_quote_async(const std::string& base_symbol, const std::string& quote_symbol);
};

#endif  // PRICE_FETCHER_H