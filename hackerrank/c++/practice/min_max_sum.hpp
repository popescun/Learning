//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <iostream>

namespace min_max_sum {
using namespace std;

/*
 * Complete the 'miniMaxSum' function below.
 *
 * The function accepts INTEGER_ARRAY arr as parameter.
 */

inline void miniMaxSum(vector<uint64_t> arr) {
  uint64_t min{numeric_limits<uint64_t>::max()}, max{0};
  for (size_t i = 0; i < arr.size(); ++i) {
    uint64_t temp{0};
    for (size_t j = 0; j < arr.size(); ++j) {
      if (i == j) {
        continue;
      }
      temp += arr[j];
    }
    if (temp < min) {
      min = temp;
    }
    if (temp > max) {
      max = temp;
    }
  }
  cout << min << " " << max << endl;
}

inline void test() {
  const vector<uint64_t> arr = {5, 6, 7, 8, 9};
  miniMaxSum(arr);
}

} // namespace min_max_sum
