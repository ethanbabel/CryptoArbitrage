// #include <gtest/gtest.h>
// #include "price_fetcher.h"

// class PriceFetcherTest : public ::testing::Test {
//     protected:
//         PriceFetcher fetcher;
//         std::string apiKey;
    
//         void SetUp() override {
//             apiKey = getenv("dRPC_API_KEY"); 
//             fetcher.setApiKey(apiKey); 
    
//             // ✅ Add token pairs in SetUp
//             fetcher.addPair("WETH", "USDT");
//             fetcher.addPair("BTC", "WETH");
//             fetcher.addPair("USDC", "DAI");
//         }
// };

// // ✅ **Test Single Swap Quote Retrieval**
// TEST_F(PriceFetcherTest, GetSwapQuoteAsync) {
//     auto future_result = fetcher.get_swap_quote_async("WETH", "USDT");
//     auto result = future_result.get();  // Wait for async result

//     EXPECT_EQ(std::get<0>(result), "WETH");
//     EXPECT_EQ(std::get<1>(result), "USDT");
//     EXPECT_GT(std::get<2>(result), 0);  // Price should be positive
// }

// // ✅ **Test Batch Swap Quotes Retrieval**
// TEST_F(PriceFetcherTest, FetchPricesAsyncBatch) {
//     int batchSize = 2;
//     auto results = fetcher.fetchPricesAsyncBatch(batchSize);

//     EXPECT_EQ(results.size(), batchSize);
    
//     for (const auto& res : results) {
//         EXPECT_FALSE(std::get<0>(res).empty());
//         EXPECT_FALSE(std::get<1>(res).empty());
//         EXPECT_GT(std::get<2>(res), 0);
//     }
// }

// // ✅ **Test Adding Token Pairs**
// TEST_F(PriceFetcherTest, AddPair) {
//     fetcher.addPair("WETH", "BTC");
//     fetcher.addPair("BNB", "USDT");

//     auto results = fetcher.fetchPricesAsyncBatch(2);
//     EXPECT_EQ(results.size(), 2);
// }

// // ✅ **Test dRPC High-Speed Batch Requests**
// TEST_F(PriceFetcherTest, HighSpeedBatchRequest) {
    
//     // Add 10 token pairs
//     fetcher.addPair("WETH", "USDC");
//     fetcher.addPair("WETH", "USDT");
//     fetcher.addPair("WETH", "WBTC");
//     fetcher.addPair("USDC", "DAI");
//     fetcher.addPair("USDC", "USDT");
//     fetcher.addPair("WBTC", "USDT");
//     fetcher.addPair("WBTC", "DAI");
//     fetcher.addPair("UNI", "WETH");
//     fetcher.addPair("LINK", "WETH");
//     fetcher.addPair("MKR", "WETH");

//     auto results = fetcher.fetchPricesAsyncBatch(10);
    
//     EXPECT_EQ(results.size(), 10);
//     for (const auto& res : results) {
//         EXPECT_GT(std::get<2>(res), 0);
//     }
// }

// // ✅ **Test dRPC Response Time**
// TEST_F(PriceFetcherTest, CheckLatency) {
//     auto start = std::chrono::high_resolution_clock::now();
//     auto results = fetcher.fetchPricesAsyncBatch(3);
//     auto end = std::chrono::high_resolution_clock::now();

//     double elapsed_time = std::chrono::duration<double>(end - start).count();
//     EXPECT_LT(elapsed_time, 1.0);  // Expect response in under 1 sec
// }

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }