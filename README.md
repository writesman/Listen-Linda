# Linda Tuple Space

This project is a C++ implementation of a distributed tuple space system based on the Linda coordination model. It features a multi-threaded TCP server and a command-line client that allow multiple clients to concurrently and safely interact with a shared tuple space.

---

## Features

- **Distributed Coordination:** Implements the core Linda operations (`out`, `rd`, `in`) for communication between distributed processes.
- **Thread-Safe Tuple Space:** The central tuple space is fully thread-safe, using mutexes and condition variables to manage concurrent access from multiple clients.
- **Blocking Operations:** The `rd` (read) and `in` (input/remove) operations will block until a matching tuple is available in the space, ensuring clients can wait for data without busy-looping.
- **Wildcard Matching:** Supports `?` as a wildcard in `rd` and `in` patterns to match any value of any type at a given position.
- **Multi-Client Server:** The TCP server is multi-threaded, handling each client connection in a separate, detached thread for high concurrency.
- **Robust Tuple Parsing:** The server can parse tuples containing strings, integers (`int64_t`), and floating-point numbers (`double`).

---

## Prerequisites

- `g++` supporting C++17
- `make` build utility

---

## Build

To compile the `server` and `client` executables, run the following command:

```bash
make
```

This command uses the provided Makefile to build the project.

---

## Usage

1. **Run the Server**

    Start the server in a terminal. It will listen for client connections on port 8080 by default.

    ```bash
    ./server
    ```

2. **Run Client**

    Open one or more new terminals to run client commands.

    - `out`: Place a new tuple into the space

        ```bash
        ./client -out '("hello", 123, 3.14)'
        ```

    - `rd`: Read a tuple that matches a pattern without removing it. This is a blocking operation.

        ```bash
        ./client -rd '("hello", ?, ?)'
        ```

    - `in`: Read and remove a tuple that matches a pattern. This is also a blocking operation.

        ```bash
        ./client -in '("hello", ?, ?)'
        ```

### Tuple Wildcard (`?`)

- The `?` symbol acts as a **wildcard** and can only be used in tuple patterns for `-rd` and `-in` commands.
- It matches **any value** at that position, regardless of its type (`int64_t`, `double`, or `std::string`).
- The `-out` command **cannot** contain a `?`, as all tuples being placed must have fully specified values.

---

## Testing

A test script is provided to demonstrate the functionality of the tuple space, including basic operations, wildcard matching, blocking behavior, and edge cases.

To run the tests:

```bash
make test
```

This will compile the project and then execute the `test_cases.sh` script.

---

## Project Structure

- **client.cpp**: A command-line client that connects to the server to send out, rd, and in requests.
- **server.cpp**: A multi-threaded TCP server that listens for client connections, parses requests, and executes operations on the TupleSpace.
- **tuplespace.h / tuplespace.cpp**: The core implementation of the thread-safe TupleSpace class, which manages tuples and handles blocking and notification for concurrent operations.
- **test_cases.sh**: An automated bash script that runs a series of client commands to verify the correctness of the server and tuple space.
- **Makefile**: Contains rules to build the server and client executables and to run the test script.

---

## Design & Implementation

### TupleSpace

- **Data Structure**: Tuples are represented as `std::vector<std::any>`, allowing them to store mixed types (`int64_t`, `double`, `std::string`).
- **Concurrency**: Thread safety is achieved using a `std::mutex` to protect the shared tuple vector and a `std::condition_variable` to manage blocking and waking up waiting threads.
- **`out`**: The `out` operation locks the mutex, adds the tuple to the space, and then calls `notify_all()` on the condition variable to wake up any threads that may be waiting in `rd` or `in`.
- **`rd` / `in`**: These operations lock the mutex and enter a `while` loop. If no matching tuple is found, they call `cv.wait()`, which atomically unlocks the mutex and puts the thread to sleep. When notified by `out`, they wake up and re-check for a match.
- **Matching Logic**: A tuple matches a pattern if they have the same number of elements and each corresponding element either has the same type and value, or the pattern element is a wildcard (`?`). If multiple tuples match a pattern, one is chosen at random to be returned.

### Server

The server creates a listening socket on port 8080. When a new client connects, it spawns a new, detached `std::thread` to handle that client's requests, allowing it to serve multiple clients concurrently. Each thread reads commands from its client, parses the tuple string, executes the corresponding `TupleSpace` operation, and sends the result back to the client.

### Client

The client is a simple command-line tool that takes a command (`-out`, `-rd`, or `-in`) and a tuple string as arguments. It establishes a TCP connection to the server, sends the command, waits for a response, prints the response to the console, and then exits.
