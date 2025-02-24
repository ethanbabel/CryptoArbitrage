#include "price_fetcher.h"

double PriceFetcher::get_swap_quote(const string& base_symbol, const string& quote_symbol) {
    http_client client(U(BASE_URL + base_symbol + "/" + quote_symbol));
    uri_builder builder;
    builder.append_query(U("baseAsset"), U(base_symbol));
    builder.append_query(U("quoteAsset"), U(quote_symbol));

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

        if (response_json.has_field(U("data")) && response_json[U("data")].has_field(U("item")) && response_json[U("data")][U("item")].has_field(U("rate"))) {
            return std::stod(response_json[U("data")][U("item")][U("rate")].as_string());
        }
    } catch (const exception& e) {
        cerr << "❌ Exception during API request: " << e.what() << endl;
    }

    return -1;
}

void PriceFetcher::update_prices(const vector<pair<string, string>>& token_pairs) {
    prices.clear();
    vector<thread> threads;

    for (const auto& pair : token_pairs) {
        threads.emplace_back([&, pair]() {
            string token1 = pair.first;
            string token2 = pair.second;
            double price = get_swap_quote(token1, token2);

            lock_guard<mutex> lock(price_mutex);
            if (price > 0) {
                prices[token1 + "_" + token2] = price;
                cout << "✅ Price " << token1 << " → " << token2 << ": " << price << endl;
            } else {
                cout << "⚠️ Failed to fetch price for " << token1 << " → " << token2 << endl;
            }
        });

        this_thread::sleep_for(chrono::milliseconds(500));  // Rate limit handling
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
}

void PriceFetcher::print_prices() {
    cout << "\n📊 Fetched Prices:" << endl;
    for (const auto& entry : prices) {
        cout << entry.first << " : " << entry.second << endl;
    }
}

unordered_map<string, double> PriceFetcher::get_prices() {
    return prices;
}