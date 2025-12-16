//
// Created by Nicolae Popescu on 14/12/2025.
//

#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <vector>

namespace find_smallest_missing_positive_integer {
using namespace std;

/*
 * Find the Smallest Missing Positive Integer
 * Given an unsorted array of integers, find the smallest positive integer not
 * present in the array in O(n) time and O(1) extra space.
 *   Example
 *     Input
 *       orderNumbers = [3, 4, -1, 1]
 *     Output
 *       2
 *
 * Complete the 'findSmallestMissingPositive' function below.
 *
 * The function is expected to return an INTEGER.
 * The function accepts INTEGER_ARRAY orderNumbers as parameter.
 */

/**
 *  Using Visited Array - O(n) Time and O(n) Space
 */
inline int findSmallestMissingPositive_VisitedArray(vector<int> &orderNumbers) {
  const auto n = static_cast<int>(orderNumbers.size());
  std::vector visited(n, false);

  for (auto i = 0; i < n; ++i) {
    if (orderNumbers[i] > 0 && orderNumbers[i] <= n) {
      visited[orderNumbers[i] - 1] = true;
    }
  }

  for (auto j = 0; j < n; ++j) {
    if (!visited[j]) {
      return j + 1;
    }
  }

  return n + 1;
}

/**
 * Specific cycle sort that moves all positive items in range [1..n] in the
 * right position, that is number[item - 1] = item
 */
inline void cycle_sort(std::vector<int> &numbers) {
  if (numbers.empty()) {
    return;
  }
  const auto n = static_cast<int>(numbers.size());
  for (int i = 0; i < n; ++i) {
    auto &item = numbers[i];
    while (true) {
      // skip if number is in position, or is equal to the swap target, or is
      // out of range [1..n]
      if (item == i + 1 || item == numbers[item - 1] || item < 1 || item > n) {
        break;
      }
      swap(item, numbers[item - 1]);
    }
  }
}

/**
 *  Using Cycle Sort - O(n) Time and O(1) Space
 */
inline int findSmallestMissingPositive_CycleSort(vector<int> &numbers) {
  cycle_sort(numbers);

  // the first index where numbers[index] != index + 1 is the smallest missing
  // positive number with value i
  const auto n = static_cast<int>(numbers.size());
  for (auto i = 0; i < n; ++i) {
    if (numbers[i] != i + 1) {
      return i + 1;
    }
  }

  // otherwise is array size + 1
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
  Function function = findSmallestMissingPositive_VisitedArray;
  // test(function);
  function = findSmallestMissingPositive_CycleSort;
  test(function);
}

} // namespace find_smallest_missing_positive_integer
