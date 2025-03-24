#include "driver.h"
#include <iostream>
#include <cstdlib>
#include <csignal>

// Declare a global flag for shutdown
volatile std::sig_atomic_t shutdownFlag = 0;

// Signal handler function
void signalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << "\n⚠️  Caught SIGINT (Ctrl+C). Cleaning up...\n";
        shutdownFlag = 1;
    }
}

int main(int argc, char** argv) {
    // Register signal handler
    std::signal(SIGINT, signalHandler);
    std::string apiKey = getenv("dRPC_API_KEY");
    std::cout << "✅ Starting crypto arbitrage system with API key." << std::endl;

    // Check command-line arguments for the graph flag
    bool runGraphAnalytics = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--graph") {
            runGraphAnalytics = true;
        }
    }

    {
        Driver driver(apiKey, runGraphAnalytics);
        driver.start();

        // Prevent the program from exiting (simulate long-running process)
        while (!shutdownFlag) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "Exiting Driver Scope..." << std::endl;
    }
    
    std::cout << "✅ Cleanup complete. Exiting gracefully." << std::endl;
    return 0;
}