#include "driver.h"
#include <iostream>
#include <cstdlib>

int main() {
   std::string apiKey = getenv("API_KEY");
   std::cout << "✅ Starting crypto arbitrage system with API key." << std::endl;

   Driver driver(apiKey);
   driver.start();

   // Prevent the program from exiting (simulate long-running process)
   while (true) {
       std::this_thread::sleep_for(std::chrono::seconds(5));
   }

   return 0;
}