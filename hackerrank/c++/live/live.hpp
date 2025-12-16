//
// Created by Nicolae Popescu on 14/12/2025.
//

#pragma once

#include <cassert>
#include <iostream>

namespace live {

using namespace std;

/*
 * Complete the 'countResponseTimeRegressions' function below.
 *
 * The function is expected to return an INTEGER.
 * The function accepts INTEGER_ARRAY responseTimes as parameter.
 */

inline int
find_smallest_missing_positive_number_visited(const vector<int> &numbers) {
  const auto n = static_cast<int>(numbers.size());
  vector visited(n, false);

  for (int i = 0; i < n; ++i) {
    if (numbers[i] > 0 && numbers[i] <= n) {
      visited[numbers[i] - 1] = true;
    }
  }

  for (int i = 0; i < n; ++i) {
    if (!visited[i]) {
      return i + 1;
    }
  }

  return n + 1;
}

inline void cycle_sort(vector<int> &numbers) {
  if (numbers.empty()) {
    return;
  }
  const auto n = static_cast<int>(numbers.size());
  for (int i = 0; i < n; ++i) {
    auto &item = numbers[i];
    while (true) {
      // skip if number is in position, or is equal to the target swap, or is
      // out of range [1..n]
      if (item == i + 1 || numbers[item - 1] == item || item < 1 || item > n) {
        break;
      }

      swap(item, numbers[item - 1]);
    }
  }
}

inline int
find_smallest_missing_positive_number_cycle_sort(vector<int> &numbers) {
  cycle_sort(numbers);

  const auto n = static_cast<int>(numbers.size());

  int i{0};
  for (i = 0; i < numbers.size(); ++i) {
    if (numbers[i] != i + 1) {
      return i + 1;
    }
  }
  return n + 1;
}

inline void test() {
  using Function = function<int(vector<int> &)>;
  auto test = [](Function &function) {
    std::vector numbers = {2, -3, 4, 1, 1, 7};
    assert(function(numbers) == 3);
    numbers = {-1, -1, -1};
    assert(function(numbers) == 1);
    numbers = {4, 3, 2, 1, 20, 17, 12, 30, 24, 18};
    assert(function(numbers) == 5);
    numbers = {7, 3, 4, -1, 1};
    assert(function(numbers) == 2);
    numbers = {};
    assert(function(numbers) == 1);
    numbers = {1};
    assert(function(numbers) == 2);
    numbers = {0};
    assert(function(numbers) == 1);
    numbers = {1, 1};
    assert(function(numbers) == 2);
    numbers = {4,  3, 2, 1,  -2, 20, 17, 12, 13, 5, 6,
               30, 7, 8, 24, 9,  10, 11, 18, 0,  -1};
    assert(function(numbers) == 14);
    numbers = {-2, 3, 4};
    assert(function(numbers) == 1);
    numbers = {-1, -2, -1, 4};
    assert(function(numbers) == 1);
    numbers = {5, 4, 1, 0, -1, 0};
    // assert(function(numbers) == 2);
    numbers = {7, 1, 1, 2, 2, 4, 4, 0, 3, 5, 5, 5, 6, 7, 7};
    assert(function(numbers) == 8);
    numbers = {-10, 0, 0, -4};
    assert(function(numbers) == 1);
    numbers = {-1, -2, -3};
    assert(function(numbers) == 1);
    numbers = {1, 1};
    assert(function(numbers) == 2);
    numbers = {4, 3, 4};
    assert(function(numbers) == 1);
    numbers = {3, 3, 3, 4, 4, 4};
    assert(function(numbers) == 1);
    numbers = {3, 3, 4, 5};
    assert(function(numbers) == 1);
    numbers = {2, 1};
    assert(function(numbers) == 3);
  };
  Function function = find_smallest_missing_positive_number_visited;
  test(function);
  function = find_smallest_missing_positive_number_cycle_sort;
  test(function);
}
} // namespace live