#include "tuplespace.h"
#include <cstdlib>

// Add a tuple to the space
void TupleSpace::out(const Tuple& tuple_data) {
    std::unique_lock<std::mutex> lock(mtx);
    space.push_back(tuple_data);
    cv.notify_all();
}

// Read a tuple matching the pattern without removing it
TupleSpace::Tuple TupleSpace::rd(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    while (true) {
        size_t idx = findRandomMatchIndexLocked(pattern);
        if (idx != INVALID_INDEX) return space[idx];
        cv.wait(lock);
    }
}

// Read and remove a tuple matching the pattern
TupleSpace::Tuple TupleSpace::in(const Tuple& pattern) {
    std::unique_lock<std::mutex> lock(mtx);
    while (true) {
        size_t idx = findRandomMatchIndexLocked(pattern);
        if (idx != INVALID_INDEX) {
            Tuple result = std::move(space[idx]);
            space.erase(space.begin() + idx);
            return result;
        }
        cv.wait(lock);
    }
}

// Check if a single value matches a pattern, with wildcard support
bool TupleSpace::valueMatches(const Value& pattern, const Value& v) {
    if (!pattern.has_value()) return true;
    if (!v.has_value()) return false;
    if (pattern.type() != v.type()) return false;

    if (pattern.type() == typeid(int64_t)) return std::any_cast<int64_t>(pattern) == std::any_cast<int64_t>(v);
    if (pattern.type() == typeid(double)) return std::any_cast<double>(pattern) == std::any_cast<double>(v);
    if (pattern.type() == typeid(std::string)) return std::any_cast<std::string>(pattern) == std::any_cast<std::string>(v);

    return false;
}

// Check if an entire tuple matches a pattern tuple
bool TupleSpace::tupleMatches(const Tuple& pattern, const Tuple& t) {
    if (pattern.size() != t.size()) return false;
    for (size_t i = 0; i < pattern.size(); ++i)
        if (!valueMatches(pattern[i], t[i])) return false;
    return true;
}

// Return a random matching tuple index, or INVALID_INDEX if none
size_t TupleSpace::findRandomMatchIndexLocked(const Tuple& pattern) {
    std::vector<size_t> matches;
    for (size_t i = 0; i < space.size(); ++i)
        if (tupleMatches(pattern, space[i])) matches.push_back(i);

    return matches.empty() ? INVALID_INDEX : matches[rand() % matches.size()];
}
