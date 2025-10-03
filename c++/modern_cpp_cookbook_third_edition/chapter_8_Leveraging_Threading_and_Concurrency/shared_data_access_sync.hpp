//
// Created by Nicolae Popescu on 29/09/2025.
//

#pragma once

#include <mutex>

#include "utils.hpp"

namespace shared_data_access_sync {

using namespace utils;

/**
 *Single shared resource. Notice here `std::cout` is shared and sync ensures
 * the messages are not mixed up.
 *
 * 1. Define a mutex in the appropriate context (class or global scope).
 * 2. Acquire a lock on this mutex before accessing the shared resource in each thread.
 */

// step 1
std::mutex g_mutex;

inline void thread_func () {
  using namespace std::chrono_literals;


  {
    // step 2
    std::lock_guard<std::mutex> lock{ g_mutex };

    print_time ();
    std::cout << "running thread " << std::this_thread::get_id () << std::endl;
  }

  std::this_thread::yield ();
  std::this_thread::sleep_for (1s);

  {
    // step 2
    std::lock_guard<std::mutex> lock{ g_mutex };
    print_time ();
    std::cout << "done in thread " << std::this_thread::get_id () << std::endl;
  }
}

inline void test_single_shared_resource() {
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back (thread_func);
  }

  for (auto& t : threads) {
    t.join ();
  }
}

/**
 * Multiple shared resource. Notice here `std::cout` is shared and sync ensures
 * the messages are not mixed up.
 *
 * 1. Define a mutex for each shared resource in the appropriate context (global or class scope).
 * 2. Lock the mutexes at the same time using a deadlock avoidance algorithm with std::lock().
 * 3. After locking them, adopt the ownership of each mutex into an std::lock_guard class to
 *    ensure they are safely released at the end of the function (or scope):
 */

// step 1
template <typename T>
struct container {
  std::mutex mutex;
  std::vector<T> data;
};

template <typename T>
void move_between_no_deadlock(container<T>& c1, container<T>& c2, T const value) {
  // step 2 -> it does an atomic locking of all mutexes
  std::lock (c1.mutex, c2.mutex);

  // step 3
  std::lock_guard<std::mutex> l1{ c1.mutex, std::adopt_lock };
  std::lock_guard<std::mutex> l2{ c2.mutex, std::adopt_lock };

  c1.data.erasse(std::remove(c1.data.begin(), c1.data.end(), value), c1.data.end());
  c2.data.push_back(value);
}

template <typename T>
void move_between_possible_deadlock(container<T>& c1, container<T>& c2, T const value) {
  // step 2 is missing

  // step 3
  std::lock_guard<std::mutex> l1{ c1.mutex, std::adopt_lock };
  std::cout << std::this_thread::get_id() <<  " first container mutex is locked" << std::endl;
  using namespace std::chrono_literals;

  std::this_thread::yield();
  std::this_thread::sleep_for(2s);

  std::lock_guard<std::mutex> l2{ c2.mutex, std::adopt_lock };
  std::cout << std::this_thread::get_id() << " second container mutex is locked" << std::endl;

  // std::remove moves elem that needs to be removed at the end and returns an iterator
  // pointing to beginning of the range that needs to be deleted with a std::erase
  auto new_end = std::remove(c1.data.begin(), c1.data.end(), value);
  // actual removing
  c1.data.erase(new_end, c1.data.end());
  c2.data.push_back(value);
}

inline void test_multiple_shared_resource() {
  container<int> c1;
  c1.data.push_back (1);
  c1.data.push_back (2);
  c1.data.push_back (3);

  container<int> c2;
  c2.data.push_back (4);
  c2.data.push_back (5);
  c2.data.push_back (6);

  std::thread t1(move_between_possible_deadlock<int>, std::ref(c1), std::ref(c2), 3);
  std::thread t2(move_between_possible_deadlock<int>, std::ref(c2), std::ref(c1), 6);

  t1.join ();
  t2.join ();

  auto print_container = [](const container<int>& c) {
    for (const auto& e : c.data) {
      std::cout << e;
    }
    std::cout << std::endl;
  };

  print_container (c1);
  print_container (c2);

  // std::thread t3(move_between_possible_deadlock<int>, std::ref(c1), std::ref(c2), 2);
  // std::thread t4(move_between_possible_deadlock<int>, std::ref(c2), std::ref(c1), 5);
  // t3.join ();
  // t4.join ();
  //
  // print_container (c1);
  // print_container (c2);
}

inline void test () {
  // test_single_shared_resource();
  test_multiple_shared_resource();
}
} // namespace shared_data_access_sync
