#include <iostream>
#include <cassert>
#include "price_fetcher.h"


void testFetchPrice(std::string apiKey) {
    PriceFetcher fetcher;
    fetcher.setApiKey(apiKey);
    std::string token1 = "USDT";
    std::string token2 = "ETH"; 

    double price = fetcher.get_swap_quote(token1, token2);
    std::cout << "price = " << std::to_string(price) << std::endl;
    assert(price > 0);  // Ensure price is a valid positive number

    std::cout << "✅ testFetchPrice PASSED, USDT<-->ETH = " << std::to_string(price) << std::endl;
}

void testHandleInvalidToken(std::string apiKey) {
    PriceFetcher fetcher;
    fetcher.setApiKey(apiKey);
    std::string token1 = "0xINVALIDTOKEN";
    std::string token2 = "ETH"; // ETH

    double price = fetcher.get_swap_quote(token1, token2);
    assert(price == -1);  // Expect failure handling

    std::cout << "✅ testHandleInvalidToken PASSED" << std::endl;
}

int main() {
    const char* apiKeyEnv = std::getenv("API_KEY");
    if (!apiKeyEnv) {
        std::cerr << "❌ ERROR: API_KEY environment variable not set. Exiting." << std::endl;
        return 1;
    }

    std::string apiKey = apiKeyEnv;

    testFetchPrice(apiKey);
    testHandleInvalidToken(apiKey);
    std::cout << "🎉 ALL TESTS PASSED!\n";
    return 0;
}