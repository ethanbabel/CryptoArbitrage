# Compiler and Flags
CC = g++
CONSERVATIVE_FLAGS = -std=c++11 -Wall -Wextra -pedantic
DEBUGGING_FLAGS = -g -O0
CFLAGS = $(CONSERVATIVE_FLAGS) $(DEBUGGING_FLAGS)

# Executable names
MAIN_EXEC = crypto_arbitrage
TEST_PRICE_EXEC = test_price_fetcher
TEST_SES_EXEC = test_ses

# Source files
MAIN_SRCS = main.cpp price_fetcher.cpp ses.cpp
TEST_PRICE_SRCS = test_price_fetcher.cpp price_fetcher.cpp
TEST_SES_SRCS = test_ses.cpp ses.cpp

# Object files
MAIN_OBJS = $(MAIN_SRCS:.cpp=.o)
TEST_PRICE_OBJS = $(TEST_PRICE_SRCS:.cpp=.o)
TEST_SES_OBJS = $(TEST_SES_SRCS:.cpp=.o)

# Libraries
LIBS = -L/usr/local/lib64 \
       -laws-cpp-sdk-sesv2 \
       -laws-cpp-sdk-core \
       -laws-cpp-sdk-email \
       -lboost_system \
       -lcrypto -lssl -lcpprest

# Default target (build all executables)
all: $(MAIN_EXEC) $(TEST_PRICE_EXEC) $(TEST_SES_EXEC)

# Compile main program
$(MAIN_EXEC): $(MAIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJS) $(LIBS)

# Compile unit test for price fetcher
$(TEST_PRICE_EXEC): $(TEST_PRICE_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_PRICE_OBJS) $(LIBS)

# Compile unit test for SES
$(TEST_SES_EXEC): $(TEST_SES_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_SES_OBJS) $(LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f *.o $(MAIN_EXEC) $(TEST_PRICE_EXEC) $(TEST_SES_EXEC)