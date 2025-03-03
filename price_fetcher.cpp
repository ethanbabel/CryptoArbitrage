#include "price_fetcher.h"

// Set the API key explicitly
void PriceFetcher::setApiKey(const string& apiKey) {
    API_KEY = apiKey;
}

// Add a token pair to be tracked by this fetcher
void PriceFetcher::addPair(const string& base, const string& quote) {
    tokenPairs.push_back(make_pair(base, quote));
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
            cerr << "Response: " << response.to_string() << endl;
            return -1;
        }

        json::value response_json = response.extract_json().get();

        if (response_json.has_field(U("data")) && response_json[U("data")].has_field(U("item")) 
            && response_json[U("data")][U("item")].has_field(U("rate"))) {
                
            // std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return std::stod(response_json[U("data")][U("item")][U("rate")].as_string());
        }
    } catch (const exception& e) {
        cerr << "❌ Exception during API request: " << e.what() << endl;
    }

    // std::this_thread::sleep_for(std::chrono::milliseconds(50)); 

    return -1;
}

// Fetch latest prices for assigned token pairs
tuple<string, string, double> PriceFetcher::fetchNextPrice() {
    if (API_KEY.empty()) {
        cerr << "❌ ERROR: Cannot fetch prices, API key is not set!" << endl;
        return {};
    }

    tuple<string, string, double> results;
    pair<string, string> pair = tokenPairs[currentPairIndex];
    currentPairIndex = (currentPairIndex + 1) % tokenPairs.size();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000/API_RATE_LIMIT)); // API rate limit handling
    double price = get_swap_quote(pair.first, pair.second);
    if (price > 0) {
        results = make_tuple(pair.first, pair.second, price);
    }


    return results;
}