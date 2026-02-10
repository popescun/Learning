//
// Created by Nicolae Popescu on 22/09/2025.
//

#pragma once

#include <expected>
#include <iostream>
#include <sstream>
#include <type_traits>

// TODO: move this file to chapter 9

/*
 * Value categories(VC) are a taxonomy(classification) of expressions.
 * They are not categories of objects, variables or types.
 *
 *  - lvalue simply means an object that has an identifiable location in memory
 * (i.e. having an address).
 *  - rvalue simply means, an object that has no identifiable location in memory
 * (i.e. having an address)
 *
 * references: - https://en.cppreference.com/w/cpp/language/value_category.html
 *             -
 * https://www.geeksforgeeks.org/c/lvalue-and-rvalue-in-c-language/
 *
 * As implementation we can use `overload resolution` to detect value category
 * of an expression.
 */

namespace understanding_value_categories {
/**
 * Matches lvalue references
 */
template <typename T> inline bool is_lvalue(T &) { return true; }

/**
 * Matches rvalue references
 */
template <typename T> inline bool is_lvalue(T &&) { return false; }

/**
 * Matches rvalue references
 */
template <typename T> inline bool is_rvalue(T &&a) {
  return std::forward<T>(!is_lvalue(a));
}

// custom implementations, copied from stl
template <typename T> struct add_lvalue_reference { // since C++ 11
  using type = T &;
};

template <typename T>
using add_lvalue_reference_t = add_lvalue_reference<T>::type; // since C++ 14

template <typename T> struct add_rvalue_reference { // since C++ 11
  using type = T &&;
};

template <typename T>
using add_rvalue_reference_t = add_rvalue_reference<T>::type; // since C++ 14

inline void test() {
  int a = 10;
  const int b = 20;
  int *ptr = &a;
  int arr[5] = {1, 2, 3};
  std::string str{"string"};
  auto lambda = []() {};

  std::stringstream ss;

  // using custom templates
  ss << "Expression 'a' is " << (is_lvalue(a) ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'a' address: " << std::addressof(a) << "\n";
  ss << "Expression 'b' is " << (is_lvalue(b) ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'ptr' is " << (is_lvalue(ptr) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'arr' is " << (is_lvalue(arr) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'a + b' is " << (is_lvalue(a + b) ? "lvalue" : "rvalue")
     << "\n";
  // ss << "Expression 'a + b' address: " << std::addressof(a + b) << "\n"; //
  // fail
  ss << "Expression '10' is " << (is_lvalue(10) ? "lvalue" : "rvalue") << "\n";
  // ss << "Expression '10' address: " << std::addressof(10) << "\n"; // fail
  ss << "Expression 'str' is " << (is_lvalue(str) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'std::string(\"Hello\")' is "
     << (is_lvalue(std::string("Hello")) ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'arr[0]' is " << (is_lvalue(arr[0]) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'lambda' is " << (is_lvalue(lambda) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression '[](){}' is " << (is_lvalue([]() {}) ? "lvalue" : "rvalue")
     << "\n";
  std::cout << ss.str();

  // using stl templates
  ss.str("");
  using T = int;
  using T_lref = std::add_lvalue_reference_t<T>;
  using T_lref_custom = add_lvalue_reference_t<T>;
  using T_rref = std::add_rvalue_reference_t<T>;
  using T_rref_custom = add_rvalue_reference_t<T>;

  ss << "\nExpression 'T' is "
     << (std::is_lvalue_reference_v<T> ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'T_ref' is "
     << (std::is_lvalue_reference_v<T_lref> ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'T_ref_custom' is "
     << (std::is_lvalue_reference_v<T_lref_custom> ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'T_rref' is "
     << (std::is_lvalue_reference_v<T_rref> ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'T_rref_custom' is "
     << (std::is_lvalue_reference_v<T_rref_custom> ? "lvalue" : "rvalue")
     << "\n";
  std::cout << ss.str();

  std::expected<std::int32_t, std::string> e;
  std::cout << "e: " << e.value_or(5) << std::endl;
}
} // namespace understanding_value_categories
