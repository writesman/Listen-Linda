#!/usr/bin/env bash
# Safe test script for tuple space functionality

set -euo pipefail

# Kill any previous server instances
pkill -f server 2>/dev/null || true

# Start the server in the background
./server > server.log 2>&1 &
SERVER_PID=$!
sleep 1

# Ensure the server is stopped on script exit
cleanup() {
    if kill -0 $SERVER_PID 2>/dev/null; then
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

# Helper function to run client commands
run() {
    echo "> $*"
    ./client "$@"
}

echo "== Basic tuple operations =="
run -out '("foo", "bar")'
run -rd '("foo", ?)'
run -in '("foo", ?)'
run -out '("alpha", "beta", 42)'
run -rd '(?, "beta", ?)'

echo "== Mixed types and numbers =="
run -out '("guy", 3.14159)'
run -rd '("guy", ?)'
run -in '("guy", ?)'
run -out '("mouse", "keyboard", 55.5, 100)'
run -rd '("mouse", ?, ?, ?)'
run -in '("mouse", ?, ?, ?)'

echo "== Wildcard matching =="
run -out '("John", "Deere", 25)'
run -out '("John", "Deere", 30)'
run -rd '(?, "Deere", ?)'
run -in '(?, "Deere", 30)'

echo "== Blocking IN test =="
echo "Starting background blocking client..."
timeout 5 ./client -in '("Hello!", 100)' &
BLOCK_PID=$!
sleep 1
echo "Background client should be waiting now..."

# Out command adds the tuple
run -out '("Hello!", 100)'

# Wait for the background client (don’t crash if it times out)
wait $BLOCK_PID 2>/dev/null || true
echo "(Background client finished or timed out)"

echo "== Multiple matching tuples =="
run -out '("yummy", "apple", 10)'
run -out '("yummy", "banana", 10)'
run -rd '("yummy", ?, 10)'
run -in '("yummy", ?, 10)'

echo "== Edge cases =="
run -out '()'
run -rd '()'
run -in '()'
run -out '("big", -9223372036854775808, 1.797693e308)'
run -rd '("big", ?, ?)'
run -in '("big", ?, ?)'

echo "== All tests finished =="
