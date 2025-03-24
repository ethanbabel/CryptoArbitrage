#include "api_throttler.h"

APIThrottler::APIThrottler() {
    lastLogTime = std::chrono::high_resolution_clock::now();
    requestCount = 0;
}

void APIThrottler::increment() {
    requestCount++;
}

void APIThrottler::logRPS() {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastLogTime).count();

    if (elapsed >= 1) {
        int count = requestCount.exchange(0);  // Reset counter atomically
        std::cout << "📊 Global API RPS: " << count / elapsed << " req/sec" << std::endl;
        lastLogTime = now;
    }
}