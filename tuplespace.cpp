#include "tuplespace.h"
#include <random>
#include <string>
#include <stdexcept>

// ----------------- Public API -----------------

void TupleSpace::out(const Tuple& tuple_data) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        space.push_back(tuple_data);
    }
    cv.notify_all(); // wake up waiting threads
}

TupleSpace::Tuple TupleSpace::rd(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]{ return findMatchIndexLocked(pattern) != INVALID_INDEX; });
    return space[findRandomMatchIndexLocked(pattern)];
}

TupleSpace::Tuple TupleSpace::in(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]{ return findMatchIndexLocked(pattern) != INVALID_INDEX; });

    size_t idx = findRandomMatchIndexLocked(pattern);
    Tuple result = std::move(space[idx]);
    space.erase(space.begin() + idx);
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
    std::lock_guard<std::mutex> lock(mtx);
    return findMatchIndexLocked(pattern) != INVALID_INDEX;
}

// ----------------- Index-based match helpers -----------------

size_t TupleSpace::findMatchIndexLocked(const Tuple& pattern) {
    for (size_t i = 0; i < space.size(); ++i) {
        if (tupleMatches(pattern, space[i])) return i;
    }
    return INVALID_INDEX;
}

size_t TupleSpace::findRandomMatchIndexLocked(const Tuple& pattern) {
    std::vector<size_t> matches_idx;
    for (size_t i = 0; i < space.size(); ++i) {
        if (tupleMatches(pattern, space[i])) matches_idx.push_back(i);
    }

    if (matches_idx.empty()) return INVALID_INDEX;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, matches_idx.size() - 1);
    return matches_idx[dist(rng)];
}
