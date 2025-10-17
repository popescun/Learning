//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <iostream>

namespace count_elements_greater_than_previous_average {

using namespace std;

/*
 * Complete the 'countResponseTimeRegressions' function below.
 *
 * The function is expected to return an INTEGER.
 * The function accepts INTEGER_ARRAY responseTimes as parameter.
 */

inline int countResponseTimeRegressions(vector<int> responseTimes) {
  uint64_t sum = 0;
  int count = 0;
  int index = -1;
  for (const auto& element : responseTimes) {
    ++index;
    uint64_t average = 0;
    if (index > 0) {
      average = sum / index;
    }
    cout << index << ":" << average << endl;
    if ( index > 0 && element > average) {
      ++count;
    }
    sum += element;
  }
  return count;
}

inline void test() {
  cout << countResponseTimeRegressions({10, 20, 30, 40}) << endl;
  // cout << countResponseTimeRegressions({100}) << endl;
}
} // namespace count_elements_greater_than_previous_average
