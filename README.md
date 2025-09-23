# Assignment 1 - Listen Linda

CSC-4770-001 - Distributed & Cloud Computing  
Marcus Writesman

## Overview

This project implements a distributed tuple space system in C++ that supports coordination between remote clients using the Linda model. It includes a TCP server and a command-line client, allowing multiple clients to interact with a shared tuple space safely.

## Prerequisites

- `g++` supporting C++17
- `make` utility

## Build

```bash
make
```

This compiles the `server` and `client` programs.

### Run Server

```bash
./server
```

Starts the server on port 8080 by default.

## Run Client

```bash
./client -out '("foo","bar")'
./client -rd '("foo",?)'
./client -in '("foo",?)'
```

- `-out` inserts a tuple
- `-rd` reads a tuple without removing it
- `-in` removes a tuple from the space

### Tuple Wildcard (`?`)

- The `?` symbol acts as a **wildcard** in tuple patterns.
- It can only be used in `-rd` and `-in` commands.
- It matches **any value** at that position, regardless of type.
- Examples:

```bash
# Read a tuple where the second element is "bar" and first/third can be anything
./client -rd '(?, "bar", ?)'
```

- `out` commands **cannot** contain `?`; tuples must have fully specified values.

## Run Tests

```bash
make test
```

This script runs a set of predefined tests demonstrating:

- Basic tuple operations
- Mixed types and numbers
- Wildcard matching
- Blocking operations
- Edge cases (empty tuples, large numbers)

## Project Structure

```text
Listen-Linda/
├── client.cpp        # Command-line client for interacting with the tuple space
├── server.cpp        # TCP server implementing the Linda tuple space
├── tuplespace.h      # TupleSpace class definition
├── tuplespace.cpp    # TupleSpace class implementation
├── test_cases.sh     # Bash script to test functionality and edge cases
├── Makefile          # Build script for compiling server and client
└── README.md         # Project documentation (this file)
```

- **client.cpp** – Sends `-out`, `-rd`, and `-in` commands to the server and prints responses.
- **server.cpp** – Handles TCP connections, parses commands, and interacts with the tuple space.
- **tuplespace.h / tuplespace.cpp** – Implements the core thread-safe tuple space with blocking operations and wildcard support.
- **test_cases.sh** – Demonstrates the tuple space functionality, including blocking, wildcards, and edge cases.
- **Makefile** – Compiles the client and server, and runs the tests.

## Design

### Tuple Space

- Tuples are `std::vector<std::any>` elements: `int64_t`, `double`, `std::string`, or wildcard (`?`).
- Operations:  
  - `out(tuple)` – store a tuple and notify waiting threads.
  - `rd(pattern)` – return a matching tuple without removing it (blocks if none).
  - `in(pattern)` – remove and return a matching tuple (blocks if none).
- Thread-safe via `std::mutex` and `std::condition_variable`.
- Wildcards match any value; matching requires the same size and type.
- Multiple matches: one tuple is chosen randomly.

### Server

- Listens on TCP (default port 8080); each client handled in a detached thread.
- Workflow: receive command → parse tuple → execute operation → return result.
- Helpers: `recvLine`, `sendLine`, `trim`, `parse_tuple`, `tuple_to_output`.

### Client

- CLI tool for sending commands (`-out`, `-rd`, `-in`) to the server.
- Connects via TCP, sends the command, prints the response.

### Concurrency & Blocking

- `rd` and `in` block until a matching tuple is available.
- `out` notifies all waiting threads.
- Multiple clients can perform blocking operations simultaneously without deadlock.

### Error Handling

- Malformed tuples or invalid commands return an error string.
- Wildcards only allowed in `rd` and `in`.
