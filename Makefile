CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# Files
SERVER_SRC = server.cpp tuplespace.cpp
CLIENT_SRC = client.cpp

# Executables
SERVER_BIN = server
CLIENT_BIN = client

.PHONY: all clean test

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: all
	./test_cases.sh

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
