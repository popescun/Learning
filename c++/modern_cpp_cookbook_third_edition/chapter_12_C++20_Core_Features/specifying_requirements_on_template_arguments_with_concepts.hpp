//
// Created by Nicolae Popescu on 22/09/2025.
//

#pragma once

#include <concepts>
#include <iostream>
#include <type_traits>

namespace specifying_requirements_on_template_arguments_with_concepts {
/**
 * A concept is a named set of constraints and a constraint is a requirement for
 * a template argument. These are used to select the appropriate function
 * overloads and template specialization.
 */

// template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>,
// T>> struct NumericalValue {
//   T value;
// };
//
// template <typename T> NumericalValue<T> wrap(T value) { return {value}; }
//
// template <typename T> T unwrap(NumericalValue<T> t) { return t.value; }

struct IComparableToInt {
  virtual bool CompareTo(int const o) = 0;
};

struct IConvertibleToInt {
  virtual bool ConvertTo(int const o) = 0;
};

template <typename T>
concept IntComparable = std::is_base_of_v<IComparableToInt, T>;

template <typename T>
concept IntConvertible = std::is_base_of_v<IConvertibleToInt, T>;

template <typename T>
concept IntComparableAndConvertible = IntComparable<T> && IntConvertible<T>;

// three print overloads with different constraints
template <IntComparable T> void print(T o) {
  std::cout << o.value << std::endl;
}

template <IntConvertible T> void print(T o) {
  std::cout << o.value << std::endl;
}

// another way of setting constraints on a template argument
template <typename T> requires IntComparableAndConvertible<T> void print(T o) {
  std::cout << o.value << std::endl;
}

template <typename T>
concept Addable = requires(T a, T b) { a + b; };

template <Addable T> T add(T a, T b) { return a + b; }

template <class T>
// concept Numerical = std::is_arithmetic_v<T>; // from <type_traits>
concept Numerical =
    std::integral<T> || std::floating_point<T>; // from <concepts>

template <Numerical  T> struct NumericalValueWithConcepts {
  T value;

  T operator+(NumericalValueWithConcepts other) { return value + other.value; }
};


template <typename T> NumericalValueWithConcepts<T> wrap(T value) {
  return {value};
}
template <typename T> T unwrap(NumericalValueWithConcepts<T> t) {
  return t.value;
}

inline void test() {
  auto nv = wrap(42);
  std::cout << nv.value << "\n";

  auto v = unwrap(nv);
  std::cout << v << "\n";

  auto wv = wrap(42.5f);
  std::cout << unwrap(wv) << "\n";

  using namespace std::string_literals;
  // auto ns = wrap("42"s); // error

  add(1, 2);
  add("1"s, "2"s);

  NumericalValueWithConcepts<int> a{1};
  NumericalValueWithConcepts<int> b{2};
  // add(a, b); // it's missing + operator
}
} // namespace specifying_requirements_on_template_arguments_with_concepts
