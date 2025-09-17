#include "tuplespace.h"
#include <random>
#include <stdexcept>
#include <string>

void TupleSpace::out(const std::vector<std::any>& tuple_data) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        tuples.push_back(tuple_data);
    }
    cv.notify_all(); // wake up waiting threads
}

std::vector<std::any> TupleSpace::rd(const std::vector<std::any>& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]{ return anyMatch(pattern); });
    return randomMatch(pattern); // return a random matching tuple
}

std::vector<std::any> TupleSpace::in(const std::vector<std::any>& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]{ return anyMatch(pattern); });

    // find all matching tuples
    std::vector<size_t> matches_idx;
    for (size_t i = 0; i < tuples.size(); i++) {
        if (matches(tuples[i], pattern)) matches_idx.push_back(i);
    }

    // pick one at random
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, matches_idx.size() - 1);
    size_t chosen = matches_idx[dist(rng)];
    std::vector<std::any> result = tuples[chosen];
    tuples.erase(tuples.begin() + chosen); // remove it

    return result;
}

bool TupleSpace::matches(const std::vector<std::any>& tuple, const std::vector<std::any>& pattern) {
    if (tuple.size() != pattern.size()) return false;

    for (size_t i = 0; i < tuple.size(); i++) {
        if (!pattern[i].has_value()) continue; // wildcard
        if (tuple[i].type() != pattern[i].type()) return false;

        if (tuple[i].type() == typeid(int64_t) &&
            std::any_cast<int64_t>(tuple[i]) != std::any_cast<int64_t>(pattern[i]))
            return false;

        if (tuple[i].type() == typeid(double) &&
            std::any_cast<double>(tuple[i]) != std::any_cast<double>(pattern[i]))
            return false;

        if (tuple[i].type() == typeid(std::string) &&
            std::any_cast<std::string>(tuple[i]) != std::any_cast<std::string>(pattern[i]))
            return false;
    }
    return true;
}

bool TupleSpace::anyMatch(const std::vector<std::any>& pattern) {
    for (const auto& t : tuples) {
        if (matches(t, pattern)) return true;
    }
    return false;
}

std::vector<std::any> TupleSpace::randomMatch(const std::vector<std::any>& pattern) {
    std::vector<size_t> matches_idx;
    for (size_t i = 0; i < tuples.size(); i++) {
        if (matches(tuples[i], pattern)) matches_idx.push_back(i);
    }

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, matches_idx.size() - 1);
    return tuples[matches_idx[dist(rng)]];
}