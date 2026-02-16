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
 * Matches lvalue.
 *
 * Note: It does not tell if it's reference or not. Use std::is_reference or
 *       std::is_lvalue_reference_v
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

// https://en.cppreference.com/w/cpp/language/value_category.html
namespace cpprefererence {
   template <class T> struct is_prvalue : std::true_type {};
   template <class T> struct is_prvalue<T&> : std::false_type {};
   template <class T> struct is_prvalue<T&&> : std::false_type {};

   template <class T> struct is_lvalue : std::false_type {};
   template <class T> struct is_lvalue<T&> : std::true_type {};
   template <class T> struct is_lvalue<T&&> : std::false_type {};

   template <class T> struct is_xvalue : std::false_type {};
   template <class T> struct is_xvalue<T&> : std::false_type {};
   template <class T> struct is_xvalue<T&&> : std::true_type {};
}

inline void test() {
  int a = 10;
  int &a_ref = a;
  const int b = 20;
  const int &b_ref = b;
  int *ptr = &a;
  int arr[5] = {1, 2, 3};
  std::string str{"string"};
  auto lambda = []() {};

  std::stringstream ss;

  // using custom templates
  ss << std::boolalpha;
  ss << "Expression 'a' is " << (is_lvalue(a) ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'a' is reference: "
     << std::is_reference_v<decltype(a)> << "\n";
  ss << "Expression 'a_ref' is " << (is_lvalue(a_ref) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'a_ref' is reference: "
     << std::is_reference<decltype(a_ref)>::value << "\n";
  ss << "Expression 'a' is lvalue ref: "
     << std::is_lvalue_reference_v<decltype(a)> << "\n";
  ss << "Expression 'a_ref' is lvalue ref: "
     << std::is_lvalue_reference_v<decltype(a_ref)> << "\n";
  ss << "Expression 'a' address: " << std::addressof(a) << "\n";
  ss << "Expression 'b' is " << (is_lvalue(b) ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'ptr' is " << (is_lvalue(ptr) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'arr' is " << (is_lvalue(arr) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'a + b' is " << (is_lvalue(a + b) ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'a + b' is reference: "
     << std::is_reference_v<decltype(a + b)> << "\n";
  ss << "Expression 'a + b' is rvalue reference: "
     << std::is_rvalue_reference_v<decltype(a + b)> << "\n";
  ss << "Expression 'a + b' is reference: "
     << std::is_reference_v<decltype(std::move(a + b))> << "\n";
  ss << "Expression 'a_ref+ b_ref' is reference: "
     << std::is_reference_v<decltype(a_ref + b_ref)> << "\n";
  // ss << "Expression 'a + b' address: " << std::addressof(a + b) << "\n";
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

  // using stl functions: note that they evaluate only type expressions
  ss.str("");
  using T = int;
  using T_lref = std::add_lvalue_reference_t<T>;
  using T_lref_custom = add_lvalue_reference_t<T>;
  using T_rref = std::add_rvalue_reference_t<T>;
  using T_rref_custom = add_rvalue_reference_t<T>;

  ss << "\nExpression 'T' is reference: " << std::is_reference_v<T>  << "\n";
  ss << "Expression 'T_lref' is "
     << (std::is_lvalue_reference_v<T_lref> ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'T_lref_custom' is "
     << (std::is_lvalue_reference_v<T_lref_custom> ? "lvalue" : "rvalue")
     << "\n";
  ss << "Expression 'T_rref' is "
     << (std::is_lvalue_reference_v<T_rref> ? "lvalue" : "rvalue") << "\n";
  ss << "Expression 'T_rref_custom' is "
     << (std::is_lvalue_reference_v<T_rref_custom> ? "lvalue" : "rvalue")
     << "\n";
  std::cout << ss.str();

   ss.str("");
   int x = 5;
   // x is not affected by std::move
   int&& rref = std::move(x);
   ss << "rref is: " << (is_lvalue(rref) ? "lvalue" : "rvalue") << "\n";
   // Note: std::move does not affect rref nor its value, but it calls function with move semantic.
   ss << "rref with std::move is: " << (is_lvalue(std::move(rref)) ? "lvalue" : "rvalue") << "\n";
   ss << "x with std::move is: " << (is_lvalue(std::move(x)) ? "lvalue" : "rvalue") << "\n";
   ss << "rref address: " << std::addressof(rref) << "\n";
   ss << "rref type is rvalue reference: " << std::is_rvalue_reference_v<decltype(rref)> << "\n";
   std::cout << ss.str();

  std::expected<std::int32_t, std::string> e;
  std::cout << "e: " << e.value_or(5) << std::endl;
}
} // namespace understanding_value_categories
