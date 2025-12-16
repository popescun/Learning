//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <iostream>
#include <vector>

namespace cycle_sort {

using namespace std;

inline void cycle_sort(std::vector<int> &numbers) {
  if (numbers.empty()) {
    return;
  }
  int writes{0};
  for (int i = 0; i < numbers.size() - 1; ++i) {
    int position{i};
    int item = numbers[i];
    while (true) {
      // count the positions where current item is greater than the number
      for (int j = position + 1; j < numbers.size(); ++j) {
        if (item > numbers[j]) {
          ++position;
        }
      }

      // if there are no numbers less than the item on the right of `i`
      // then stop this cycle and store the item if it is different then the
      // number at this position
      if (position == i) {
        if (item != numbers[i]) {
          numbers[i] = item;
          writes += 1;
        }
        break;
      }

      // ignore all duplicate elements
      while (item == numbers[position]) {
        ++position;
      }

      // swap number at this position and item if they are different
      if (item != numbers[position]) {
        swap(numbers[position], item);
        writes += 2;
      }

      // continue this cycle with the new item
      position = i;
    }
  }

  // cout << "writes:" << writes << endl;
}

inline void cycle_sort_2(std::vector<int> &numbers) {
  if (numbers.empty()) {
    return;
  }
  int writes{0};
  const auto n = static_cast<int>(numbers.size());
  for (int i = 0; i < n - 1; ++i) {
    auto item = numbers[i];
    auto pos = i;
    for (int j = i + 1; j < n; ++j) {
      if (item > numbers[j]) {
        ++pos;
      }
    }

    if (pos == i) {
      continue;
    }

    while (item == numbers[pos]) {
      ++pos;
    }

    // if (pos != i) { //todo: this might be redundant?
    swap(item, numbers[pos]);
    writes += 2;
    // }

    // rotate rest of the cycle until there is no need for moving
    while (pos != i) {
      pos = i;

      for (int j = i + 1; j < n; ++j) {
        if (item > numbers[j]) {
          ++pos;
        }
      }

      while (item == numbers[pos]) {
        ++pos;
      }

      if (item != numbers[pos]) {
        swap(item, numbers[pos]);
        writes += 2;
      }
    }
  }

  cout << "writes:" << writes << endl;
}

inline void test() {
  std::vector<int> numbers = {10, 5, 2, 2, 3};
  auto print = [&numbers]() {
    for (const auto &n : numbers) {
      cout << n << " ";
    }
    cout << endl;
  };

  // cycle_sort(numbers);
  // print();
  numbers = {10, 5, 2, 2, 3};
  cycle_sort_2(numbers);
  print();

  numbers = {20, 40, 40, 50, 50, 10, 10, 30};
  cycle_sort(numbers);
  print();
  numbers = {20, 40, 40, 50, 50, 10, 10, 30};
  cycle_sort_2(numbers);
  print();

  numbers = {10, 9, 8, 7, 4, 5, 3, 2, 1, 0};
  cycle_sort(numbers);
  print();
  numbers = {10, 9, 8, 7, 4, 5, 3, 2, 1, 0};
  cycle_sort_2(numbers);
  print();

  numbers = {};
  cycle_sort(numbers);
  print();
  numbers = {};
  cycle_sort_2(numbers);
  print();
}
} // namespace cycle_sort
