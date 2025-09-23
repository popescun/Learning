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

inline void func1() {
  std::cout << "thread func1 without params" << std::endl;
};

inline void func2(int i, double d, const std::string& s) {
  std::cout << "thread func2 with params " << i <<", " << d << ", " << s << std::endl;
};


inline void create_thread() {

  std::thread t;
  t = std::thread([]() {
    std::cout << "create thread after declaration" << std::endl;
  });

  std::thread t1(func1);
  std::thread t2([]() {
    std::cout << "thread lambda without params" << std::endl;
  });
  std::thread t3(func2, 42, 42.0, "42");

  t.join();
  t1.join();
  t2.join();
  t3.join();
}

inline void test() {
  print_time();
  create_thread();
}
} // namespace threading_and_concurrency
