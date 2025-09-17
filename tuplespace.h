#pragma once

#include <vector>
#include <any>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstdint>

class TupleSpace {
public:
    using Value = std::any;            // Single value in a tuple
    using Tuple = std::vector<Value>;  // A tuple is a vector of values

    // Public API
    void out(const Tuple& tuple_data); // Write a tuple to the space
    Tuple rd(const Tuple& pattern);    // Read a tuple without removing it
    Tuple in(const Tuple& pattern);    // Read and remove a tuple

    // Helper for checking wildcards
    static bool isWildcard(const Value& v);

private:
    // Sentinel for "no match"
    static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

    // Internal storage
    std::vector<Tuple> space;
    std::mutex mtx;
    std::condition_variable cv;

    // Matching helpers
    static bool valueMatches(const Value& pattern, const Value& v);
    static bool tupleMatches(const Tuple& pattern, const Tuple& t);

    // Index-based helper
    size_t findRandomMatchIndexLocked(const Tuple& pattern);
};
