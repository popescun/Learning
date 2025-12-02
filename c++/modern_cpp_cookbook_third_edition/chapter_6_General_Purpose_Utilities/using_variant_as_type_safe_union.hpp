//
// Created by Nicolae Popescu on 25/11/2025.
//

#pragma once

#include <iostream>
#include <string>
#include <variant>

/**
 * A variant is a type-safe union that holds the value of one of its
 * alternative types.
 *
 * Introduced in C++ 17.
 */
namespace using_variant_as_type_safe_union {

struct foo {
  int value;
  explicit foo (const int v) : value (v) {
  }
};

/**
 * From book: Variants are intended to be used for holding alternatives of
 * similar non-polymorphic and non-POD types !? See below how foo and bar are
 * both accepted.
 */
struct bar : public foo {};

inline void using_variant () {
  // 1. set and get
  std::variant<int, double, std::string, foo, bar> v{ 42 }; // holds int
  // always get the actual stored type
  std::cout << std::get<int> (v) << std::endl;
  // using index of actual type
  std::cout << std::get<0> (v) << std::endl; // same
  v.emplace<std::string> ("hello");
  // set using assignment
  v = "hello";
  std::cout << std::get<std::string> (v) << std::endl;
  std::cout << std::get<2> (v) << std::endl; // same
  v.emplace<foo> (43);                       // holds foo
  std::cout << std::get<foo> (v).value << std::endl;
  std::cout << std::get<3> (v).value << std::endl; // same

  // 2. trying to get different type than actual stored will throw
  try {
    std::cout << std::get<int> (v) << std::endl;
  } catch (const std::bad_variant_access& e) {
    std::cout << e.what () << std::endl;
  }

  // 3. get the index of actual stored type
  static_assert (std::variant_size_v<decltype (v)> == 5);
  std::cout << "index:" << v.index () << std::endl;
  v = 42;
  std::cout << "index:" << v.index () << std::endl;

  // 4. checks if a variant currently holds a given type
  std::cout << "int? " << std::boolalpha << std::holds_alternative<int> (v) << std::endl;
  std::cout << "foo? " << std::boolalpha << std::holds_alternative<foo> (v) << std::endl;
  v = "hello";
  std::cout << "string? " << std::boolalpha
            << std::holds_alternative<std::string> (v) << std::endl;
}

inline void test () {
  using_variant ();
}
} // namespace using_variant_as_type_safe_union
