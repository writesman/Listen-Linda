#!/bin/bash

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

# Stop the server
kill $SERVER_PID
echo "Server stopped"
