#ifndef API_THROTTLER_H
#define API_THROTTLER_H

#include <atomic>
#include <chrono>
#include <iostream>
#include <iostream>
#include <chrono>

class APIThrottler {
public:
    APIThrottler();
    void increment();
    void logRPS(); 
    
private:
    std::atomic<int> requestCount;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastLogTime;
};

#endif // API_THROTTLER_H