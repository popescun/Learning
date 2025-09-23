//
// Created by Nicolae Popescu on 22/09/2025.
//

#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace threading_and_concurrency {

inline void print_time() {
  const auto now = std::chrono::system_clock::now();
  const auto stime = std::chrono::system_clock::to_time_t(now);
  const auto ltime = std::localtime(&stime);

  std::stringstream ss;
  ss << std::put_time(ltime, "%c") << '\n';
  std::cout << ss.str() << std::endl;
}

  void func1() {

};

inline void create_thread() {

  std::thread t;

  std::thread t1
}

inline void test() {
  print_time();
}
} // namespace using_structured_bindings_to_handle_multi_return_values
