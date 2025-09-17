#pragma once

#include <vector>
#include <any>
#include <mutex>
#include <condition_variable>

class TupleSpace {
private:
    std::vector<std::vector<std::any>> tuples;
    std::mutex mtx;
    std::condition_variable cv;

public:
    // Add a tuple to the space
    void out(const std::vector<std::any>& tuple_data);

    // Read a tuple matching the pattern (blocking)
    std::vector<std::any> rd(const std::vector<std::any>& pattern);

    // Read and remove a tuple matching the pattern (blocking)
    std::vector<std::any> in(const std::vector<std::any>& pattern);

private:
    // Check if a tuple matches a pattern (wildcards allowed)
    bool matches(const std::vector<std::any>& tuple, const std::vector<std::any>& pattern);

    // Check if any tuple matches a pattern
    bool anyMatch(const std::vector<std::any>& pattern);

    // Return a random matching tuple (without removing)
    std::vector<std::any> randomMatch(const std::vector<std::any>& pattern);
};
