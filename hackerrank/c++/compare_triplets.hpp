//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <iostream>

namespace compare_triplets {
using namespace std;

/*
 * Complete the 'compareTriplets' function below.
 *
 * The function is expected to return an INTEGER_ARRAY.
 * The function accepts following parameters:
 *  1. INTEGER_ARRAY a
 *  2. INTEGER_ARRAY b
 */

inline vector<int> compareTriplets(vector<int> a, vector<int> b) {
  vector<int> result = {0, 0};
  for (auto i = 0; i < a.size(); ++i) {
    result[0] += a[i] > b[i] ? 1 : 0;
    result[1] += a[i] < b[i] ? 1 : 0;
  }
  return result;
}

inline void test() {
  const auto a = {5, 6, 7};
  const auto b = {3, 6, 10};
  const auto result = compareTriplets(a, b);
  cout << result[0] << "," << result[1] << endl;
}

} // namespace compare_triplets
