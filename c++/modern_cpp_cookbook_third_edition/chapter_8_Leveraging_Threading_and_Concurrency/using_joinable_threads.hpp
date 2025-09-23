//
// Created by Nicolae Popescu on 22/09/2025.
//

#pragma once

#include <iostream>
#include <thread>

namespace using_joinable_threads {
inline void thread_func(int i) {
  while (--i > 0) {
    std::cout << i << std::endl;
  }
}

inline void test() {
  std::jthread t(thread_func, 10);
}
} // namespace using_joinable_threads
