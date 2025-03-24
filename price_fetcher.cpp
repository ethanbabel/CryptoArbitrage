#include "price_fetcher.h"

// Atomic counter for API request tracking
std::atomic<int> requestCount(0);
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

// Constructor for PriceFetcher
PriceFetcher::PriceFetcher() {
    // Load token data
    const std::string tokenInfoPath = "token_info.csv";
    std::ifstream tokenFile(tokenInfoPath);
    if (!tokenFile.is_open()) {
        throw std::runtime_error("❌ ERROR: Could not open token info file: " + tokenInfoPath);
    }

    std::string line, symbol, address, decimals;
    std::getline(tokenFile, line);  // Skip header

    while (std::getline(tokenFile, line)) {
        std::stringstream ss(line);
        if (std::getline(ss, symbol, ',') && std::getline(ss, address, ',') && std::getline(ss, decimals, ',')) {
            symbol = trim(symbol);
            address = trim(address);
            decimals = trim(decimals);

            tokenAddresses[symbol] = address;
            tokenDecimals[symbol] = std::stoi(decimals);
        }
    }

    tokenFile.close();
    std::cout << "✅ Loaded " << tokenAddresses.size() << " tokens from " << tokenInfoPath << std::endl;

    // Load fee tiers and available token pairs
    const std::string feeTierPath = "token_fee_tiers.csv";
    std::ifstream feeFile(feeTierPath);
    if (!feeFile.is_open()) {
        throw std::runtime_error("❌ ERROR: Could not open token fee tiers file: " + feeTierPath);
    }

    std::string baseSymbol, quoteSymbol, feeTier;
    std::getline(feeFile, line);  // Skip header

    while (std::getline(feeFile, line)) {
        std::stringstream ss(line);
        if (std::getline(ss, baseSymbol, ',') && std::getline(ss, quoteSymbol, ',') && std::getline(ss, feeTier, ',')) {
            baseSymbol = trim(baseSymbol);
            quoteSymbol = trim(quoteSymbol);
            feeTier = trim(feeTier);

            int fee = std::stoi(feeTier);

            // Store the fee tier for the pair
            tokenFeeTiers[baseSymbol][quoteSymbol] = fee;
        }
    }

    feeFile.close();
    std::cout << "✅ Loaded " << tokenFeeTiers.size() << " token fee tiers from " << feeTierPath << std::endl;
}

// Set the API key explicitly
void PriceFetcher::setApiKey(const string& apiKey) {
    API_KEY = apiKey;
}

// Set the API throttler for tracking request rate
void PriceFetcher::setThrottler(APIThrottler* throttlerPtr) {
    throttler = throttlerPtr;
}

// Encode a Uniswap V3 Quote Request in JSON-RPC format
json::value PriceFetcher::encodeQuoteRequest(const std::string& base, const std::string& quote, uint256_t amount) {
    json::value requestBody;
    requestBody[U("jsonrpc")] = json::value::string(U("2.0"));
    requestBody[U("id")] = json::value::number(1);
    requestBody[U("method")] = json::value::string(U("eth_call"));

    json::value paramsObj;
    paramsObj[U("to")] = json::value::string(U(UNISWAP_V3_QUOTER));  // Uniswap V3 Quoter Contract

    // Construct ABI-encoded function call
    std::string data = encodeABI(base, quote, amount);
    paramsObj[U("data")] = json::value::string(U(data));

    json::value paramsArray = json::value::array();
    paramsArray[0] = paramsObj;
    paramsArray[1] = json::value::string(U("latest"));

    requestBody[U("params")] = paramsArray;
    return requestBody;
}

// Helper function: Pad a hex string to 64 characters with leading zeros
std::string padLeft(const std::string& input, size_t length = 64) {
    if (input.size() >= length) return input;  // No padding needed
    return std::string(length - input.size(), '0') + input;
}

std::string PriceFetcher::encodeABI(const std::string& baseSymbol, const std::string& quoteSymbol, uint256_t amount) {
    auto baseIt = tokenAddresses.find(baseSymbol);
    auto quoteIt = tokenAddresses.find(quoteSymbol);

    if (baseIt == tokenAddresses.end() || quoteIt == tokenAddresses.end()) {
        throw std::runtime_error("Invalid token symbol provided: " + baseSymbol + " or " + quoteSymbol);
    }

    std::string baseAddress = baseIt->second;
    std::string quoteAddress = quoteIt->second;

    // Determine the correct fee tier
    int feeTier = 3000;  // Default to 0.3% if not found
    auto baseTierIt = tokenFeeTiers.find(baseSymbol);
    if (baseTierIt != tokenFeeTiers.end()) {
        auto quoteTierIt = baseTierIt->second.find(quoteSymbol);
        if (quoteTierIt != baseTierIt->second.end()) {
            feeTier = quoteTierIt->second;
        }
    }

    std::stringstream encoded;
    encoded << "0xf7729d43";  // Function selector for `quoteExactInputSingle()`

    encoded << padLeft(baseAddress.substr(2), 64);  // Remove "0x"
    encoded << padLeft(quoteAddress.substr(2), 64);

    // Convert fee tier to hex and pad it
    std::stringstream feeHex;
    feeHex << std::hex << feeTier;
    encoded << padLeft(feeHex.str(), 64);

    // Convert amount to hex and pad it
    std::stringstream amountHex;
    amountHex << std::hex << amount;
    encoded << padLeft(amountHex.str(), 64);

    encoded << padLeft("0", 64);  // sqrtPriceLimitX96 (0 = no limit)

    return encoded.str();
}

// Fetch a swap quote asynchronously via dRPC
pplx::task<std::tuple<std::string, std::string, double>> PriceFetcher::get_swap_quote_async(
    const std::string& base_symbol, const std::string& quote_symbol) {

    // trackApiRate(); // Update API request rate tracker
    if (throttler) throttler->increment();  // Global RPS tracker

    http_client client(U(dRPC_URL + API_KEY));
    http_request request(methods::POST);
    request.headers().add(U("Content-Type"), U("application/json"));

    // Fetch correct amountIn based on base token decimals
    int base_decimals = tokenDecimals.count(base_symbol) ? tokenDecimals[base_symbol] : 18;
    uint256_t amountIn = uint256_t(1) * uint256_t(static_cast<uint64_t>(pow(10, base_decimals - 2)));  // Use 0.01 of the base token

    json::value requestBody = encodeQuoteRequest(base_symbol, quote_symbol, amountIn);
    request.set_body(requestBody);

    return client.request(request).then([=](pplx::task<http_response> previous_task) {
        try {
            http_response response = previous_task.get();
            auto raw_response = response.extract_utf8string().get();
    
            if (response.status_code() != status_codes::OK) {
                cerr << "⚠️ dRPC request failed for " << base_symbol << " → " << quote_symbol
                     << " | Status Code: " << response.status_code() << std::endl;
                return std::make_tuple(base_symbol, quote_symbol, -1.0);
            }
    
            json::value response_json = json::value::parse(raw_response);
            if (response_json.has_field(U("result"))) {
                double price = std::stod(response_json[U("result")].as_string()) / pow(10, base_decimals);
    
                // std::cout << "✅ Price fetched: " << base_symbol << " → " << quote_symbol << " = " << price << std::endl;
    
                return std::make_tuple(base_symbol, quote_symbol, price);
            } else {
                std::cout << "⚠️ No 'result' field in dRPC response for " << base_symbol << " → " << quote_symbol << ". " << std::endl;
                std::cout << "🔹 Request Body: " << requestBody.serialize() << std::endl;
                std::cout << "🔹 Response: " << raw_response << std::endl;
            }
    
        } catch (const std::exception& e) {
            cerr << "❌ Exception during dRPC request: " << e.what() << std::endl;
            return std::make_tuple(base_symbol, quote_symbol, -1.0);
        }
    
        return std::make_tuple(base_symbol, quote_symbol, -1.0);
    });
}

// Fetch multiple swap quotes asynchronously using batch requests
std::vector<std::tuple<std::string, std::string, double>> PriceFetcher::fetchPricesAsyncBatch(int batchSize,
    const std::vector<std::pair<std::string, std::string>>& tokenPairs) {
    
    static thread_local size_t currentPairIndex = 0;

    std::vector<pplx::task<std::tuple<std::string, std::string, double>>> tasks;
    std::vector<std::tuple<std::string, std::string, double>> results;

    for (int i = 0; i < batchSize && !tokenPairs.empty(); ++i) {
        const auto& pair = tokenPairs[currentPairIndex];
        currentPairIndex = (currentPairIndex + 1) % tokenPairs.size();
        tasks.push_back(get_swap_quote_async(pair.first, pair.second));
    }

    auto all_tasks = pplx::when_all(tasks.begin(), tasks.end());
    auto all_results = all_tasks.get();

    for (const auto& result : all_results) {
        results.push_back(result);
    }

    return results;
}