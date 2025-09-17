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

    // Helper for checking wildcards
    static bool isWildcard(const Value& v);

private:
    // Internal storage
    std::vector<Tuple> tuples;
    std::mutex mtx;
    std::condition_variable cv;

    // Matching helpers
    static bool valueMatches(const Value& pattern, const Value& v);
    static bool tupleMatches(const Tuple& pattern, const Tuple& t);

    // Internal helper functions
    bool anyMatch(const Tuple& pattern);
    Tuple randomMatch(const Tuple& pattern);
};
