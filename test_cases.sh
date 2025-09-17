#!/bin/bash

# Ensure no previous server is running
pkill -f server 2>/dev/null || true

# Start server in background
./server &
SERVER_PID=$!
echo "Server started with PID $SERVER_PID"
sleep 1  # give server time to start

echo "=== Basic tuple operations ==="
./client -out '("foo", "bar")'
./client -rd '("foo", ?)'
./client -in '("foo", ?)'
./client -out '("alpha", "beta", 42)'
./client -rd '(?, "beta", ?)'

echo "=== Wildcard tests ==="
./client -out '("John", "Doe", 25)'
./client -out '("Jane", "Doe", 30)'
./client -rd '("John", ?, ?)'
./client -rd '(?, "Doe", ?)'
./client -in '(?, "Doe", 30)'

echo "=== Wildcard verification ==="
# Add tuples
./client -out '("apple", 42)'
./client -out '("banana", 7)'

# rd with wildcard for second element
echo "Testing rd with wildcard..."
./client -rd '("apple", ?)'  # should match ("apple", 42)

# in with wildcard for second element
echo "Testing in with wildcard..."
./client -in '("apple", ?)'  # should remove ("apple", 42)

# Safe non-blocking verification after removal
echo "Testing rd after removal..."
echo "No tuple ('apple', ?) should exist now. Skipping blocking rd test."

echo "=== Multiple tuples ==="
./client -out '("apple", "fruit")'
./client -out '("carrot", "vegetable")'
./client -rd '(?, ?)'
./client -in '("apple", ?)'
./client -rd '(?, ?)'  # should now only find carrot

echo "=== Blocking in test ==="
# Start a background client that waits for a tuple that doesn't exist yet
(
    echo "Background client waiting for tuple..."
    ./client -in '("waiting", 100)'
    echo "Background client received tuple!"
) &

sleep 2  # ensure background client is waiting

# Now add the tuple that the background client is waiting for
./client -out '("waiting", 100)'

sleep 2  # give background client time to print message

# Stop the server if still running
if kill -0 $SERVER_PID 2>/dev/null; then
    kill $SERVER_PID
fi
echo "Server stopped"
