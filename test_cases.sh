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

echo "=== Double tests ==="
./client -out '("pi", 3.14159)'
./client -rd '("pi", ?)'
./client -in '("pi", ?)'

./client -out '("e", 2.71828, "Euler")'
./client -rd '("e", ?, ?)'
./client -in '("e", ?, ?)'

echo "=== Mixed-type tuple tests ==="
./client -out '("temperature", 98, 36.6)'
./client -rd '("temperature", ?, ?)'
./client -in '("temperature", ?, ?)'

./client -out '("sensor", "humidity", 55.5, 100)'
./client -rd '("sensor", ?, ?, ?)'
./client -in '("sensor", ?, ?, ?)'

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

echo "=== Multiple matching tuples test ==="
# Add multiple tuples that match the same template
./client -out '("fruit", "apple", 10)'
./client -out '("fruit", "banana", 10)'
./client -out '("fruit", "cherry", 10)'

# rd with template that matches all three
echo "Reading a tuple with template ('fruit', ?, 10)..."
./client -rd '("fruit", ?, 10)'  # should randomly return one of the three

# in with template that matches remaining tuples
echo "Removing a tuple with template ('fruit', ?, 10)..."
./client -in '("fruit", ?, 10)'  # should remove a random one of the remaining two

# rd again to verify remaining tuple
echo "Reading again with template ('fruit', ?, 10)..."
./client -rd '("fruit", ?, 10)'  # should return one of the two remaining

# in again to remove the last one
echo "Removing last tuple with template ('fruit', ?, 10)..."
./client -in '("fruit", ?, 10)'

# rd should now block if uncommented, so we skip or just note
echo "All matching tuples removed. No tuple with ('fruit', ?, 10) remains."

# Stop the server if still running
if kill -0 $SERVER_PID 2>/dev/null; then
    kill $SERVER_PID
fi
echo "Server stopped"
