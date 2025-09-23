//
// Created by Nicolae Popescu on 22/09/2025.
//

#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace threading_basic {

inline void print_time() {
  const auto now = std::chrono::system_clock::now();
  const auto stime = std::chrono::system_clock::to_time_t(now);
  const auto ltime = std::localtime(&stime);

  std::stringstream ss;
  ss << std::put_time(ltime, "%c") << '\n';
  std::cout << ss.str();
}

inline void func1() {
  std::cout << "thread func1 without params" << std::endl;
};

inline void func2(int i, double d, const std::string &s) {
  std::cout << "thread func2 with params " << i << ", " << d << ", " << s
            << std::endl;
};

inline void func3(int &i) { i *= 2; }

inline void func4() {
  std::cout << "func4..." << std::endl;
  using namespace std::chrono;
  print_time();
  std::cout << "sleeping for 2 seconds" << std::endl;
  std::this_thread::sleep_for(2s);
  print_time();
}

inline void func5() {
  std::cout << "func5..." << std::endl;
  using namespace std::chrono;
  print_time();
  std::cout << "sleeping until" << std::endl;
  std::this_thread::sleep_until(std::chrono::system_clock::now() + 2s);
  print_time();
}

inline void func6(std::chrono::seconds timeout) {
  std::cout << "func6..." << std::endl;
  const auto then = std::chrono::system_clock::now() + timeout;
  do {
    // std::cout << "yield..." << std::endl;
    std::this_thread::yield();
  } while (std::chrono::system_clock::now() < then);
}

inline void create_thread() {
  std::thread t;
  t = std::thread(
      []() { std::cout << "create thread after declaration" << std::endl; });

  std::thread t1(func1);
  std::thread t2(
      []() { std::cout << "thread lambda without params" << std::endl; });
  std::thread t3(func2, 42, 42.0, "42");

  std::cout << "this thread id:" << std::this_thread::get_id() << std::endl;

  t.join();
  t1.join();
  t2.join();
  t3.join();

  // detach
  std::thread t4([]() { std::cout << "thread detached" << std::endl; });
  std::cout << t4.joinable() << std::endl;
  t4.detach(); // is not guaranteed will terminate before the process terminates
               // we need a way to sync this
  std::cout << t4.joinable()
            << std::endl; // a detach thread is no longer joinable

  int a = 5;
  std::thread t5(func3, std::ref(a));
  std::cout << "a:" << a << std::endl;
  t5.join();
  std::cout << "a:" << a << std::endl;

  std::thread t6(func4);
  t6.join();

  std::thread t7(func5);
  t7.join();

  std::thread t8(func6, std::chrono::seconds(2));
  t8.join();
  print_time();
}

inline void test() {
  print_time();
  create_thread();
}
} // namespace threading_and_concurrency
