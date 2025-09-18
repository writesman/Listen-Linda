CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# Files
SERVER_SRC = server.cpp tuplespace.cpp
CLIENT_SRC = client.cpp

# Executables
SERVER_BIN = server
CLIENT_BIN = client
TEST_SCRIPT = test_cases.sh

.PHONY: all clean test

all: $(SERVER_BIN) $(CLIENT_BIN) make_test_executable

$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

make_test_executable:
	chmod +x $(TEST_SCRIPT)

test: all
	./$(TEST_SCRIPT)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
