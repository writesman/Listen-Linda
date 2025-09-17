#include "tuplespace.h"
#include <cstdlib>   // for rand()

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

    for (;;) {  // loop until a random match is found
        size_t idx = findRandomMatchIndexLocked(pattern);
        if (idx != INVALID_INDEX) {
            return space[idx]; // return a copy without removing
        }
        cv.wait(lock);
    }
}

TupleSpace::Tuple TupleSpace::in(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);

    for (;;) {  // loop until a random match is found
        size_t idx = findRandomMatchIndexLocked(pattern);
        if (idx != INVALID_INDEX) {
            Tuple result = std::move(space[idx]); // move for efficiency
            space.erase(space.begin() + idx);     // remove from space
            return result;
        }
        cv.wait(lock);
    }
}

// ------------------- Helper functions -------------------

bool TupleSpace::isWildcard(const Value& v) {
    return !v.has_value();
}

bool TupleSpace::valueMatches(const Value& pattern, const Value& v) {
    if (isWildcard(pattern)) return true;
    if (!pattern.has_value() || !v.has_value()) return false;

    const std::type_info& pType = pattern.type();
    const std::type_info& vType = v.type();
    if (pType != vType) return false;

    if (pType == typeid(int64_t))
        return std::any_cast<int64_t>(pattern) == std::any_cast<int64_t>(v);
    else if (pType == typeid(double))
        return std::any_cast<double>(pattern) == std::any_cast<double>(v);
    else if (pType == typeid(std::string))
        return std::any_cast<std::string>(pattern) == std::any_cast<std::string>(v);
    else
        return false; // unsupported type
}

bool TupleSpace::tupleMatches(const Tuple& pattern, const Tuple& t) {
    if (pattern.size() != t.size()) return false;

    for (size_t i = 0; i < pattern.size(); ++i) {
        if (!valueMatches(pattern[i], t[i])) return false;
    }
    return true;
}

// ------------------- Index-based helper -------------------

size_t TupleSpace::findRandomMatchIndexLocked(const Tuple& pattern) {
    std::vector<size_t> matches;
    matches.reserve(space.size());

    for (size_t i = 0; i < space.size(); ++i) {
        if (tupleMatches(pattern, space[i])) matches.push_back(i);
    }
    if (matches.empty()) return INVALID_INDEX;

    return matches[rand() % matches.size()];
}
