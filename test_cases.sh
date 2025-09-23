#!/usr/bin/env bash
# Test script for Linda tuple space

set -euo pipefail

# Ensure the server is stopped on script exit
cleanup() {
    kill "$SERVER_PID" 2>/dev/null || true
    wait "$SERVER_PID" 2>/dev/null || true
}
trap cleanup EXIT

# Run client commands
run() {
    echo "> $*"
    ./client "$@"
}

# Start server in background
./server >server.log 2>&1 &
SERVER_PID=$!
sleep 1

# Basic tuple operations
echo "== Basic tuple operations =="
run -out '("foo", "bar")'
run -rd '("foo", ?)'
run -in '("foo", ?)'
run -out '("alpha", "beta", 42)'
run -rd '(?, "beta", ?)'

# Mixed types and numbers
echo "== Mixed types and numbers =="
run -out '("guy", 3.14159)'
run -rd '("guy", ?)'
run -in '("guy", ?)'
run -out '("mouse", "keyboard", 55.5, 100)'
run -rd '("mouse", ?, ?, ?)'
run -in '("mouse", ?, ?, ?)'

# Wildcard matching
echo "== Wildcard matching =="
run -out '("John", "Deere", 25)'
run -out '("John", "Deere", 30)'
run -rd '(?, "Deere", ?)'
run -in '(?, "Deere", 30)'

# Blocking IN test
echo "== Blocking IN test =="
echo "Starting background blocking client..."
timeout 5 ./client -in '("Hello!", 100)' &
BLOCK_PID=$!
sleep 1
echo "Background client should be waiting now..."
run -out '("Hello!", 100)'
wait "$BLOCK_PID" 2>/dev/null || true
echo "(Background client finished or timed out)"

# Multiple matching tuples
echo "== Multiple matching tuples =="
run -out '("yummy", "apple", 10)'
run -out '("yummy", "banana", 10)'
run -rd '("yummy", ?, 10)'
run -in '("yummy", ?, 10)'

# Edge cases
echo "== Edge cases =="
run -out '()'
run -rd '()'
run -in '()'
run -out '("big", -9223372036854775808, 1.797693e308)'
run -rd '("big", ?, ?)'
run -in '("big", ?, ?)'

echo "== All tests finished =="
