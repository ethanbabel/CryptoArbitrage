# Compiler and Flags
CC = g++
CONSERVATIVE_FLAGS = -std=c++11 -Wall -Wextra -pedantic
DEBUGGING_FLAGS = -g -O0
CFLAGS = $(CONSERVATIVE_FLAGS) $(DEBUGGING_FLAGS)

# Executable names
MAIN_EXEC = crypto_arbitrage
TEST_EXEC = test_price_fetcher

# Source files
MAIN_SRCS = main.cpp price_fetcher.cpp
TEST_SRCS = test_price_fetcher.cpp price_fetcher.cpp

# Object files
MAIN_OBJS = $(MAIN_SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Libraries
LIBS = -lboost_system -lcrypto -lssl -lcpprest

# Default target
all: $(MAIN_EXEC) $(TEST_EXEC)

# Compile main program
$(MAIN_EXEC): $(MAIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJS) $(LIBS)

# Compile unit test program
$(TEST_EXEC): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LIBS)

# Compile main source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f *.o $(MAIN_EXEC) $(TEST_EXEC)