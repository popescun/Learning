//
// Created by Nicolae Popescu on 29/09/2025.
//

#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>

namespace utils {
inline void print_time () {
  const auto now   = std::chrono::system_clock::now ();
  const auto stime = std::chrono::system_clock::to_time_t (now);
  const auto ltime = std::localtime (&stime);

  std::stringstream ss;
  ss << std::put_time (ltime, "%c") << '\n';
  std::cout << ss.str ();
}

} // namespace utils
