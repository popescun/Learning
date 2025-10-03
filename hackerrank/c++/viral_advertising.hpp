//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <iostream>

namespace viral_advertising {

using namespace std;

/*
 * Complete the 'viralAdvertising' function below.
 *
 * The function is expected to return an INTEGER.
 * The function accepts INTEGER n as parameter.
 */

inline int viralAdvertising(int n) {
  int shares {5};
  int total_likes{0};
  while (n--) {
    int likes = floor(shares / 2);
    shares = likes * 3;
    total_likes += likes;
  }
  return total_likes;
}

inline void test() {
  cout << viralAdvertising(3) << endl;
}
} // namespace viral_advertising
