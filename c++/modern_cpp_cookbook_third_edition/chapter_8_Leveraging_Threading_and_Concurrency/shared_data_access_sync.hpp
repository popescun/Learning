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
 *    The typical use of a mutex to protect access to a shared resource comprises locking
 *    the mutex, using the shared resource, and then unlocking the mutex:
 *
 *      #include <mutex>
 *      g_mutex.lock();
 *      // use the shared resource such as std::cout
 *      std::cout << "accessing shared resource" << '\n';
 *      g_mutex.unlock();
 *
 *      This method of using the mutex is, however, prone to error. This is because each
 *      call to lock() must be paired with a call to unlock() on all execution paths; that is,
 *      both normal return paths and exception return paths.In order to safely acquire and
 *      release a mutex, regardless of the way the execution of a function goes, the C++
 *      standard defines several locking classes:
 *
 *      - std::lock_guard(<mutex> header) is the locking mechanism seen earlier; it represents a mutex
 *      wrapper implemented in an RAII manner. It attempts to acquire the mutex at
 *      the time of its construction and release it upon destruction. This is available in C++11.
 *      It can work with recursive mutex as well.
 *
 *      - std::unique_lock(<mutex> header) is a mutex ownership wrapper that provides support for
 *      deferred locking, time locking, recursive locking, transfer of ownership, and
 *      using it with condition variables. This is available in C++11.
 *
 *      - std::shared_lock(<shared_mutex> header) is a mutex-shared ownership wrapper that provides
 *      support for deferred locking, time locking, and transfer of ownership. This is available in C++14.
 *
 *      - std::scoped_lock(<mutex> header) is a wrapper for multiple mutexes implemented in an RAII manner. Upon
 *      construction, it attempts to acquire ownership of the mutexes in a deadlock avoidance manner as if
 *      it is using std::lock(), and upon destruction, it releases the mutexes in reverse order of the way
 *      they were acquired. This is available in C++17.
 *
 */

// step 1
inline std::mutex g_mutex, g_mutex1;
inline std::recursive_mutex g_recursive_mutex;

inline void thread_func () {
  using namespace std::chrono_literals;

  // using std::lock_guard
  {
    // step 2
    std::lock_guard lock{ g_mutex }; // no multiple mutexes!
    // std::lock_guard<std::mutex> lock1{ g_mutex }; // this is a dead-lock !

    std::lock_guard<std::recursive_mutex> lock2{ g_recursive_mutex };
    std::lock_guard<std::recursive_mutex> lock3{ g_recursive_mutex }; // no dead-lock as the mutex is recursive

    print_time ();
    std::cout << "running thread with lock_guard" << std::this_thread::get_id ()
              << std::endl;
  }

  std::this_thread::yield ();
  std::this_thread::sleep_for (1s);

  // using std::unique_lock
  {
    // step 2
    std::unique_lock lock{ g_mutex }; // no multiple mutexes!
    // std::unique_lock<std::mutex> lock1{ g_mutex}; // this is a dead-lock !

    std::unique_lock<std::recursive_mutex> lock2{ g_recursive_mutex };
    std::unique_lock<std::recursive_mutex> lock3{ g_recursive_mutex }; // no dead_lock as the mutex is recursive

    std::unique_lock lock5 (g_mutex1, std::defer_lock); // deferred lock, no lock yet!
    lock5.lock (); // here g_mutex1 is locked

    print_time ();
    std::cout << "still running thread with unique_lock"
              << std::this_thread::get_id () << std::endl;
  }

  std::this_thread::yield ();
  std::this_thread::sleep_for (1s);

  // using scoped_lock
  {
    // step 2
    std::scoped_lock lock{ g_mutex, g_mutex1 }; // can lock multiple mutexes
    // std::unique_lock<std::mutex> lock1{ g_mutex}; // this is a dead-lock !

    std::scoped_lock lock2{ g_recursive_mutex };
    std::scoped_lock lock3{ g_recursive_mutex }; // no dead_lock as the mutex is recursive

    print_time ();
    std::cout << "still running thread with scoped_lock"
              << std::this_thread::get_id () << std::endl;
  }

  std::this_thread::yield ();
  std::this_thread::sleep_for (1s);

  {
    // step 2
    std::lock_guard<std::mutex> lock{ g_mutex };
    print_time ();
    std::cout << "done in thread with lock_guard" << std::this_thread::get_id ()
              << std::endl;
  }
}

inline void test_single_shared_resource () {
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back (thread_func);
  }

  for (auto& t : threads) {
    t.join ();
  }
}

/**
 * Multiple shared resource. Notice here `std::cout` is shared and sync ensures the messages are not mixed up.
 *
 * 1. Define a mutex for each shared resource in the appropriate context (global or class scope).
 * 2. Lock the mutexes at the same time using a deadlock avoidance algorithm with std::lock().
 * 3. After locking them, adopt the ownership of each mutex into a std::lock_guard class to
 *    ensure they are safely released at the end of the function (or scope):
 */

// step 1
template <typename T>
struct container {
  std::mutex mutex;
  std::vector<T> data;
};

template <typename T>
void move_between_no_deadlock (container<T>& c1, container<T>& c2, T const value) {
  // using lock_guard
  {
    // step 2 -> it does an atomic locking of all mutexes
    std::lock (c1.mutex, c2.mutex);

    // step 3
    std::lock_guard l1{ c1.mutex, std::adopt_lock };
    std::lock_guard l2{ c2.mutex, std::adopt_lock };

    auto orig_size = c1.data.size ();
    auto it        = c1.data.erase (
    std::remove (c1.data.begin (), c1.data.end (), value), c1.data.end ());
    if (orig_size != c1.data.size ()) {
      c2.data.push_back (value);
    }
  }

  // using scoped_lock
  {
    // step 2 -> it does an atomic locking of all mutexes
    std::scoped_lock lock (c1.mutex, c2.mutex);

    // just for demonstration, this reverse previous move
    auto orig_size = c2.data.size ();
    auto it        = c2.data.erase (
    std::remove (c2.data.begin (), c2.data.end (), value), c2.data.end ());
    if (orig_size != c2.data.size ()) {
      c1.data.push_back (value);
    }
  }
}

template <typename T>
void move_between_possible_deadlock (container<T>& c1, container<T>& c2, T const value) {
  // step 2 is missing

  // step 3
  using namespace std::chrono_literals;
  std::cout << std::this_thread::get_id ()
            << " first container mutex locking..." << std::endl;
  std::lock_guard l1{ c1.mutex };
  std::cout << std::this_thread::get_id () << " first container mutex is locked"
            << std::endl;

  std::this_thread::yield ();

  /**
   * This may dead-lock if c1 refer to c2 when this is executed in a second thread.
   * Notice that we don't use std::adopt_lock in order to simulate the dead-lock.
   * As explianed on https://en.cppreference.com/w/cpp/thread/lock_tag_t.html, std::adopt_lock means:
   * assume the calling thread already has ownership of the mutex.
   * It seems it actually makes the mutex to act as an recursive mutex, and therefore the dead-lock does not occur.
   * But it may be a bug in stl!?
   */
  std::cout << std::this_thread::get_id ()
            << " second container mutex locking..." << std::endl;
  std::lock_guard l2{ c2.mutex };
  std::cout << std::this_thread::get_id ()
            << " second container mutex is locked" << std::endl;

  // std::remove moves elem that needs to be removed at the end and returns an iterator
  // pointing to beginning of the range that needs to be deleted with a std::erase
  auto orig_size = c1.data.size ();
  auto new_end   = std::remove (c1.data.begin (), c1.data.end (), value);
  // actual removing
  c1.data.erase (new_end, c1.data.end ());
  if (orig_size != c1.data.size ()) {
    c2.data.push_back (value);
  }
}

inline void test_multiple_shared_resource () {
  container<int> c1;
  c1.data.push_back (1);
  c1.data.push_back (2);
  c1.data.push_back (3);

  container<int> c2;
  c2.data.push_back (4);
  c2.data.push_back (5);
  c2.data.push_back (6);

  std::thread t1 (move_between_no_deadlock<int>, std::ref (c1), std::ref (c2), 3);
  std::thread t2 (move_between_no_deadlock<int>, std::ref (c2), std::ref (c1), 6);

  t1.join ();
  t2.join ();

  auto print_container = [] (const container<int>& c) {
    for (const auto& e : c.data) {
      std::cout << e;
    }
    std::cout << std::endl;
  };

  print_container (c1);
  print_container (c2);

  // enable this to check dead-lock case
  // std::thread t3 (move_between_possible_deadlock<int>, std::ref (c1), std::ref (c2), 2);
  // std::thread t4 (move_between_possible_deadlock<int>, std::ref (c2), std::ref (c1), 5);
  //
  // t3.join ();
  // t4.join ();
}

inline void test () {
  // test_single_shared_resource();
  test_multiple_shared_resource ();
}
} // namespace shared_data_access_sync
