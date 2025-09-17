#include "tuplespace.h"
#include <random>
#include <string>
#include <stdexcept>

void TupleSpace::out(const Tuple& tuple_data) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        space.push_back(tuple_data);
    }
    cv.notify_all(); // wake up waiting threads
}

TupleSpace::Tuple TupleSpace::rd(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]{ return anyMatch(pattern); });
    return randomMatch(pattern);
}

TupleSpace::Tuple TupleSpace::in(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]{ return anyMatch(pattern); });

    // find all matching tuples
    std::vector<size_t> matches_idx;
    for (size_t i = 0; i < space.size(); i++) {
        if (tupleMatches(pattern, space[i])) matches_idx.push_back(i);
    }

    // pick one at random
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, matches_idx.size() - 1);
    size_t chosen = matches_idx[dist(rng)];
    Tuple result = space[chosen];
    space.erase(space.begin() + chosen); // remove it

    return result;
}

// ----------------- Matching helpers -----------------

bool TupleSpace::isWildcard(const Value& v) {
    return !v.has_value();
}

bool TupleSpace::valueMatches(const Value& pattern, const Value& v) {
    if (isWildcard(pattern)) return true;
    if (pattern.type() != v.type()) return false;

    if (pattern.type() == typeid(int64_t))
        return std::any_cast<int64_t>(pattern) == std::any_cast<int64_t>(v);
    if (pattern.type() == typeid(double))
        return std::any_cast<double>(pattern) == std::any_cast<double>(v);
    if (pattern.type() == typeid(std::string))
        return std::any_cast<std::string>(pattern) == std::any_cast<std::string>(v);

    return false; // unknown types
}

bool TupleSpace::tupleMatches(const Tuple& pattern, const Tuple& t) {
    if (pattern.size() != t.size()) return false;
    for (size_t i = 0; i < t.size(); ++i) {
        if (!valueMatches(pattern[i], t[i])) return false;
    }
    return true;
}

// ----------------- Other helpers -----------------

bool TupleSpace::anyMatch(const Tuple& pattern) {
    for (const auto& t : space) {
        if (tupleMatches(pattern, t)) return true;
    }
    return false;
}

TupleSpace::Tuple TupleSpace::randomMatch(const Tuple& pattern) {
    std::vector<size_t> matches_idx;
    for (size_t i = 0; i < space.size(); i++) {
        if (tupleMatches(pattern, space[i])) matches_idx.push_back(i);
    }

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, matches_idx.size() - 1);
    return space[matches_idx[dist(rng)]];
}
