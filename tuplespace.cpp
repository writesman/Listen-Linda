#include "tuplespace.h"
#include <iostream>
#include <thread>
#include <chrono>

// ------------------- Public API -------------------

void TupleSpace::out(const Tuple& tuple_data) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        space.push_back(tuple_data);
    }
    cv.notify_all(); // wake up any threads waiting for a match
}

TupleSpace::Tuple TupleSpace::rd(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this, &pattern] {
        return findMatchIndexLocked(pattern) != INVALID_INDEX;
    });

    size_t idx = findMatchIndexLocked(pattern);
    return space[idx]; // return a copy without removing
}

TupleSpace::Tuple TupleSpace::in(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this, &pattern] {
        return findMatchIndexLocked(pattern) != INVALID_INDEX;
    });

    size_t idx = findMatchIndexLocked(pattern);
    Tuple result = space[idx];
    space.erase(space.begin() + idx); // remove from space
    return result;
}

// ------------------- Helper functions -------------------

bool TupleSpace::isWildcard(const Value& v) {
    // Example: treat typeid(void) as wildcard
    return v.type() == typeid(void);
}

bool TupleSpace::valueMatches(const Value& pattern, const Value& v) {
    if (isWildcard(pattern)) return true;
    return pattern.type() == v.type() && pattern.has_value() && v.has_value() && pattern.type() == v.type();
}

bool TupleSpace::tupleMatches(const Tuple& pattern, const Tuple& t) {
    if (pattern.size() != t.size()) return false;
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (!valueMatches(pattern[i], t[i])) return false;
    }
    return true;
}

// ------------------- Index-based helpers -------------------

size_t TupleSpace::findMatchIndexLocked(const Tuple& pattern) {
    for (size_t i = 0; i < space.size(); ++i) {
        if (tupleMatches(pattern, space[i])) return i;
    }
    return INVALID_INDEX;
}

size_t TupleSpace::findRandomMatchIndexLocked(const Tuple& pattern) {
    std::vector<size_t> matches;
    for (size_t i = 0; i < space.size(); ++i) {
        if (tupleMatches(pattern, space[i])) matches.push_back(i);
    }
    if (matches.empty()) return INVALID_INDEX;

    size_t randIdx = rand() % matches.size();
    return matches[randIdx];
}
