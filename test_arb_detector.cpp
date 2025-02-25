#include "arb_detector.h"
#include <gtest/gtest.h>

// Test fixture class for ArbDetector
class ArbDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        arbDetector = new ArbDetector();
    }

    void TearDown() override {
        delete arbDetector;
    }

    ArbDetector* arbDetector;
};

// Test: Graph updates correctly with new price data
TEST_F(ArbDetectorTest, GraphUpdate) {
    arbDetector->updateGraph("BTC", "ETH", 15.0);
    arbDetector->updateGraph("ETH", "USDT", 2000.0);

    ASSERT_EQ(arbDetector->getGraph().size(), 2);  // Two nodes should exist
}

// Test: No arbitrage cycle when prices are normal
TEST_F(ArbDetectorTest, NoArbitrage) {
    arbDetector->updateGraph("BTC", "ETH", 15.0);
    arbDetector->updateGraph("ETH", "USDT", 2000.0);
    arbDetector->updateGraph("USDT", "BTC", 0.000033);  // No arbitrage

    std::vector<std::string> cycle = arbDetector->testBellmanFord("BTC");
    ASSERT_TRUE(cycle.empty());  // No arbitrage cycle found
}

// Test: Detect arbitrage cycle
TEST_F(ArbDetectorTest, DetectsArbitrage) {
    arbDetector->updateGraph("BTC", "ETH", 15.0);
    arbDetector->updateGraph("ETH", "USDT", 2000.0);
    arbDetector->updateGraph("USDT", "BTC", 0.00005);  // Arbitrage condition

    std::vector<std::string> cycle = arbDetector->testBellmanFord("BTC");
    ASSERT_FALSE(cycle.empty());  // Arbitrage cycle should be found
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}