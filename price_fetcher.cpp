#include "price_fetcher.h"

// Atomic counter for API request tracking
std::atomic<int> requestCount(0);
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

// Track and print API request rate
void trackApiRate() {
    using namespace std::chrono;
    if (requestCount == 0) {
        startTime = high_resolution_clock::now();
    }
    requestCount++;

    auto now = high_resolution_clock::now();
    double elapsedSeconds = duration<double>(now - startTime).count();

    if (elapsedSeconds >= 1.0) {
        std::cout << "📊 API Requests per Second: " << requestCount << " req/sec" << std::endl;
        requestCount = 0;
        startTime = now;
    }
}

// Set the API key explicitly
void PriceFetcher::setApiKey(const string& apiKey) {
    API_KEY = apiKey;
}

// Add a token pair to be tracked by this fetcher
void PriceFetcher::addPair(const string& base, const string& quote) {
    tokenPairs.push_back(make_pair(base, quote));
}

// Fetch a swap quote asynchronously
pplx::task<std::tuple<std::string, std::string, double>> PriceFetcher::get_swap_quote_async(
    const std::string& base_symbol, const std::string& quote_symbol) {

    if (API_KEY.empty()) {
        cerr << "❌ ERROR: API key not set before making API request!" << endl;
        return pplx::task_from_result(std::make_tuple(std::string(""), std::string(""), -1.0));
    }

    auto request_start = std::chrono::high_resolution_clock::now(); // Start timing request

    trackApiRate(); // Update API request rate tracker

    http_client client(U(BASE_URL + base_symbol + "/" + quote_symbol));
    http_request request(methods::GET);
    request.headers().add(U("Authorization"), U("Bearer " + API_KEY));
    request.headers().add(U("X-API-Key"), U(API_KEY));

    return client.request(request).then([=](pplx::task<http_response> previous_task) {
        try {
            http_response response = previous_task.get();

            if (response.status_code() != status_codes::OK) {
                cerr << "⚠️ API request failed for " << base_symbol << " → " << quote_symbol
                     << " | Status Code: " << response.status_code() << endl;
                return std::make_tuple(base_symbol, quote_symbol, -1.0);
            }

            json::value response_json = response.extract_json().get();

            if (response_json.has_field(U("data")) && response_json[U("data")].has_field(U("item")) &&
                response_json[U("data")][U("item")].has_field(U("rate"))) {
                
                auto request_end = std::chrono::high_resolution_clock::now();
                double elapsed_time = std::chrono::duration<double>(request_end - request_start).count();

                double price = std::stod(response_json[U("data")][U("item")][U("rate")].as_string());

                std::cout << "✅ Price fetched: " << base_symbol << " → " << quote_symbol 
                          << " = " << price << " (⏳ " << elapsed_time << " sec)" << std::endl;

                return std::make_tuple(base_symbol, quote_symbol, price);
            }
        } catch (const std::exception& e) {
            cerr << "❌ Exception during API request: " << e.what() << endl;
        }

        return std::make_tuple(base_symbol, quote_symbol, -1.0);
    });
}

// Fetch multiple swap quotes asynchronously
std::vector<std::tuple<std::string, std::string, double>> PriceFetcher::fetchPricesAsyncBatch(int batchSize) {
    std::vector<pplx::task<std::tuple<std::string, std::string, double>>> tasks;
    std::vector<std::tuple<std::string, std::string, double>> results;

    for (int i = 0; i < batchSize && currentPairIndex < tokenPairs.size(); ++i) {
        const auto& pair = tokenPairs[currentPairIndex];
        currentPairIndex = (currentPairIndex + 1) % tokenPairs.size();
        tasks.push_back(get_swap_quote_async(pair.first, pair.second));
    }

    // Wait for all async requests to finish
    for (auto& task : tasks) {
        results.push_back(task.get());
    }

    return results;
}