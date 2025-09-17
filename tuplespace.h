#pragma once

#include <vector>
#include <any>
#include <mutex>
#include <condition_variable>

class TupleSpace {
public:
    // Type aliases
    using Value = std::any;           // A single value in a tuple
    using Tuple = std::vector<Value>; // A tuple is a vector of values

    // Public API
    void out(const Tuple& tuple_data);       // Add a tuple to the space
    Tuple rd(const Tuple& pattern);          // Read a tuple matching the pattern (blocking)
    Tuple in(const Tuple& pattern);          // Read and remove a tuple matching the pattern (blocking)

    // Static helper
    static bool isWildcard(const Value& v);  // returns true if value is a wildcard (empty)

private:
    // Internal storage
    std::vector<Tuple> tuples;
    std::mutex mtx;
    std::condition_variable cv;

    // Internal helper functions
    bool matches(const Tuple& tuple, const Tuple& pattern);
    bool anyMatch(const Tuple& pattern);
    Tuple randomMatch(const Tuple& pattern);
};
