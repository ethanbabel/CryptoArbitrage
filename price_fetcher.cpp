#include "price_fetcher.h"

// Set the API key explicitly
void PriceFetcher::setApiKey(const string& apiKey) {
    API_KEY = apiKey;
}

// Add a token pair to be tracked by this fetcher
void PriceFetcher::addPair(const string& base, const string& quote) {
    lock_guard<mutex> lock(price_mutex);
    tokenPairs.emplace_back(base, quote);
}

// Fetch a swap quote for a given token pair
double PriceFetcher::get_swap_quote(const string& base_symbol, const string& quote_symbol) {
    if (API_KEY.empty()) {
        cerr << "❌ ERROR: API key not set before making API request!" << endl;
        return -1;
    }

    http_client client(U(BASE_URL + base_symbol + "/" + quote_symbol));

    http_request request(methods::GET);
    request.headers().add(U("Authorization"), U("Bearer " + API_KEY));
    request.headers().add(U("X-API-Key"), U(API_KEY));

    try {
        http_response response = client.request(request).get();

        if (response.status_code() != status_codes::OK) {
            cerr << "⚠️ API request failed for " << base_symbol << " → " << quote_symbol
                 << " | Status Code: " << response.status_code() << endl;
            return -1;
        }

        json::value response_json = response.extract_json().get();

        if (response_json.has_field(U("data")) && response_json[U("data")].has_field(U("item")) 
            && response_json[U("data")][U("item")].has_field(U("rate"))) {
            return std::stod(response_json[U("data")][U("item")][U("rate")].as_string());
        }
    } catch (const exception& e) {
        cerr << "❌ Exception during API request: " << e.what() << endl;
    }

    return -1;
}

// Fetch latest prices for assigned token pairs
vector<tuple<string, string, double>> PriceFetcher::fetchPrices() {
    if (API_KEY.empty()) {
        cerr << "❌ ERROR: Cannot fetch prices, API key is not set!" << endl;
        return {};
    }

    vector<tuple<string, string, double>> results;
    lock_guard<mutex> lock(price_mutex);

    for (const auto& pair : tokenPairs) {
        double price = get_swap_quote(pair.first, pair.second);
        if (price > 0) {
            results.emplace_back(pair.first, pair.second, price);
        }
    }

    return results;
}