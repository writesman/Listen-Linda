#pragma once

#include <any>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class TupleSpace {
public:
  using Value = std::any;
  using Tuple = std::vector<Value>;

  void out(const Tuple &tuple_data);
  Tuple rd(const Tuple &pattern);
  Tuple in(const Tuple &pattern);

private:
  // Sentinel value for no matching tuple found
  static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

  std::vector<Tuple> space;
  std::mutex mtx;
  std::condition_variable cv;

  static bool valueMatches(const Value &pattern, const Value &candidate);
  static bool tupleMatches(const Tuple &pattern, const Tuple &candidate);

  size_t findRandomMatchIndexLocked(const Tuple &pattern);
};
