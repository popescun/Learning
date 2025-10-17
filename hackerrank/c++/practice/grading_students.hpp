//
// Created by Nicolae Popescu on 3/10/2025.
//

#pragma once

#include <iostream>

namespace grading_students {

using namespace std;

/*
 * Complete the 'gradingStudents' function below.
 *
 * The function is expected to return an INTEGER_ARRAY.
 * The function accepts INTEGER_ARRAY grades as parameter.
 */

inline vector<int> gradingStudents(vector<int> grades) {
  vector<int> result;
  result.resize(grades.size());
  int i = -1;
  for (const auto &grade : grades) {
    ++i;
    result[i] = grade;
    if (grade < 38) {
      continue;
    }
    auto reminder = grade % 5;
    if (5 - reminder >= 3) {
      continue;
    }
    // cout << reminder << endl;
    result[i] = grade + 5 - reminder;
  }
  return result;
}

inline void test() {
  for (const auto &grade : gradingStudents({84, 29, 57})) {
    cout << grade << endl;
  }
}
} // namespace grading_students
