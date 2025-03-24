#ifndef PRICE_FETCHER_H
#define PRICE_FETCHER_H

#include "utils.h"
#include "api_throttler.h"
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

// Fix: Remove Casablanca's `U(x)` macro before including Boost
#ifdef U
#undef U
#endif

#include <boost/multiprecision/cpp_int.hpp>  // Boost Multiprecision

// Restore `U(x)` for Casablanca REST SDK
#define U(x) _XPLATSTR(x)

#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace std;

class PriceFetcher {
private:
    string API_KEY;  // API Key is now set after construction
    APIThrottler* throttler;  // Pointer to the global throttler object

    string dRPC_URL = "https://lb.drpc.org/ogrpc?network=ethereum&dkey=";  // dRPC ETH URL
    string UNISWAP_V3_QUOTER = "0xb27308f9F90D607463bb33eA1BeBb41C27CE5AB6";  // Uniswap V3 Quoter Contract
    
    std::unordered_map<std::string, std::string> tokenAddresses;  // Token symbol to address mapping
    std::unordered_map<std::string, int> tokenDecimals;  // Token symbol to decimals mapping
    std::unordered_map<std::string, std::unordered_map<std::string, int>> tokenFeeTiers;  // Token pair to fee tier mapping

public:
    PriceFetcher();  

    void setApiKey(const string& apiKey);
    void setThrottler(APIThrottler* throttlerPtr);

    std::vector<std::tuple<std::string, std::string, double>> fetchPricesAsyncBatch(int batchSize, const std::vector<std::pair<std::string, std::string>>& pairs);
    pplx::task<std::tuple<std::string, std::string, double>> get_swap_quote_async(const std::string& base_symbol, const std::string& quote_symbol);
    json::value encodeQuoteRequest(const std::string& base, const std::string& quote, uint256_t amount);
    std::string encodeABI(const std::string& baseSymbol, const std::string& quoteSymbol, uint256_t amount);
};

#endif  // PRICE_FETCHER_H