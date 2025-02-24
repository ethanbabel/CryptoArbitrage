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

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace std;

class PriceFetcher {
private:
    const string API_KEY;  // Replace with actual API key
    const string BASE_URL = "https://rest.cryptoapis.io/v2/market-data/exchange-rates/by-symbol/";
    mutex price_mutex;
    unordered_map<string, double> prices;

public:
    PriceFetcher() : API_KEY(std::getenv("API_KEY")) {
        if (API_KEY.empty()) {
            cerr << "API_KEY environment variable not set." << endl;
            exit(1);
        }
    }
    void update_prices(const vector<pair<string, string>>& token_pairs);
    void print_prices();
    unordered_map<string, double> get_prices();

    double get_swap_quote(const string& base_symbol, const string& quote_symbol);
};

#endif