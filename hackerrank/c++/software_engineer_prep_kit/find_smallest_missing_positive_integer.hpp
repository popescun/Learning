//
// Created by Nicolae Popescu on 14/12/2025.
//

#pragma once

#include <assert.h>
#include <iostream>

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

inline int findSmallestMissingPositive(vector<int> orderNumbers) {
  const auto n = static_cast<int>(orderNumbers.size());
  std::vector<bool> visited(n, false);

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

inline void test() {
  assert(findSmallestMissingPositive({-1, -1, -1}) == 1);
  assert(findSmallestMissingPositive({4, 3, 2, 1, 20, 17, 12, 30, 24, 18}) ==
         5);
  assert(findSmallestMissingPositive({7, 3, 4, -1, 1}) == 2);
  assert(findSmallestMissingPositive({}) == 1);
  assert(findSmallestMissingPositive({1}) == 2);
  assert(findSmallestMissingPositive({0}) == 1);
  assert(findSmallestMissingPositive({1, 1}) == 2);

  assert(findSmallestMissingPositive({4,  3, 2, 1,  -2, 20, 17, 12, 13, 5, 6,
                                      30, 7, 8, 24, 9,  10, 11, 18, 0,  -1}) ==
         14);
  assert(findSmallestMissingPositive({-2, 3, 4}) == 1);
  assert(findSmallestMissingPositive({-1, -2, -1, 4}) == 1);
  assert(findSmallestMissingPositive({5, 4, 1, 0, -1, 0}) == 2);
  // cout << findSmallestMissingPositive({7, 1, 1, 2, 2, 4, 4, 0, 3, 5, 5, 5, 6,
  // 7, 7}) << endl; cout << findSmallestMissingPositive({-10, 0, 0, -4}) <<
  // endl; cout << findSmallestMissingPositive({-1, -2, -3}) << endl; cout <<
  // findSmallestMissingPositive({1, 1}) << endl; cout <<
  // findSmallestMissingPositive({4, 3, 4}) << endl;

  // cout << findSmallestMissingPositive({3, 3, 3, 4, 4, 4}) << endl;

  // cout << findSmallestMissingPositive({3, 3, 4, 5}) << endl;
}

} // namespace find_smallest_missing_positive_integer
