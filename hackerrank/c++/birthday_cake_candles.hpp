//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <algorithm>
#include <iostream>

namespace birthday_cake_candles {
using namespace std;

/*
 * Complete the 'birthdayCakeCandles' function below.
 *
 * The function is expected to return an INTEGER.
 * The function accepts INTEGER_ARRAY candles as parameter.
 *
 * You are in charge of the cake for a child's birthday.
 * It will have one candle for each year of their total age.
 * They will only be able to blow out the tallest of the candles.
 * Your task is to count how many candles are the tallest.
 */

inline int birthdayCakeCandles(vector<int> candles) {
  const auto max_elem = max_element(candles.begin(), candles.end());
  return count_if(candles.begin(), candles.end(),
                  [&max_elem](const auto &e) { return e == *max_elem; });
}

inline void test() {
  const vector<int> arr = {4, 4, 1, 3, 4, 3, 4};
  cout << birthdayCakeCandles(arr) << endl;
}

} // namespace birthday_cake_candles
