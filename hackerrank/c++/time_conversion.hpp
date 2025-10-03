//
// Created by Nicolae Popescu on 19/09/2025.
//

#pragma once

#include <iostream>

namespace time_conversion {

using namespace std;

/*
 * Complete the 'timeConversion' function below.
 *
 * The function is expected to return a STRING.
 * The function accepts STRING s as parameter.
 */

inline string timeConversion(string s) {
  auto hh = s.substr(0, 2);
  const auto mm = s.substr(3, 2);
  const auto ss = s.substr(6, 2);
  const auto day_part = s.substr(8, 2);

  if (day_part == "AM") {
    // 12AM -> 11AM, 00 -> 11
    const auto h = stoi(hh);
    const auto check = h == 12 ? 0 : h;
    if ( check > 11) {
      throw std::runtime_error("Invalid hh");
    }
    hh = h == 12 ? "00" : hh;
    return hh + ":" + mm + ":" + ss;
  } else if (day_part == "PM") {
    // 12PM -> 11PM =  12 -> 23
    auto h = stoi(hh);
    if ( h != 12 && h > 11) {
      throw std::runtime_error("Invalid hh");
    }
    h = h == 12 ? h : h + 12;
    hh = to_string(h);
    return hh + ":" + mm + ":" + ss;
  }

  return {};
}

inline void test() {
  constexpr auto s {"12:40:22AM"};

  const auto result = timeConversion(s);

  std::cout << result << std::endl;
}
} // namespace time_conversion
