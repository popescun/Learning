//
// Created by Nicolae Popescu on 25/11/2025.
//

#pragma once

#include "utils.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

/**
 * A variant is a type-safe union that holds the value of
 * one of its alternative types.
 */
namespace visiting_and_variant {

enum class genre { drama, action, sf, comedy };

struct movie {
  std::string title{ "none" };
  std::chrono::minutes duration;
  std::vector<genre> genres;
};

struct track {
  std::string title{ "none" };
  std::chrono::minutes duration;
};

struct music {
  std::string title{ "none" };
  std::string artist;
  std::vector<track> tracks;
};

struct software {
  std::string title{ "none" };
  std::string vendor;
};

using Dvd  = std::variant<movie, music, software>;
using Dvds = std::vector<Dvd>;

/**
 * A void visitor that does not return anything, but has side effects. The
 * following example prints the title of each DVD to the console
 */
inline void visit_with_lambda (const Dvds& dvds) {
  auto visitor = [] (auto&& dvd) { std::cout << dvd.title << std::endl; };
  for (const auto& d : dvds) {
    std::visit (visitor, d);
  }
}

/**
 * A visitor that returns a value; the value should have the same type,
 * regardless of the current alternative of the variant, or can be itself a
 * variant. In the following example, we visit a variant and return a new
 * variant of the same type that has the title property from any of its
 * alternatives transformed to uppercase letters
 */
inline void visit_with_visitor_that_returns (const Dvds& dvds) {
  auto to_upper = [] (std::string& s) {
    std::ranges::transform (s, s.begin (), toupper);
  };
  for (auto& d : dvds) {
    Dvd visit_and_create_copy = std::visit (
    [&] (auto&& dvd) -> Dvd {
      auto copy{ dvd };
      to_upper (copy.title);
      return copy;
    },
    d);

    auto visitor = [] (auto&& dvd) { std::cout << dvd.title << std::endl; };

    std::visit (visitor, visit_and_create_copy);
  }
}

/**
 * A visitor that does type matching (which can either be a void or a
 * value-returning visitor) implemented by providing a function object that has
 * an overloaded call operator for each alternative type of the variant:
 */
inline void visit_with_map_matching_functor_visitor (const Dvds& dvds) {
  struct visitor {
    void operator() (const movie& m) const {
      std::cout << "\nMovie"
                << "\ntitle: " << m.title
                << "\nduration: " << m.duration.count () << " min" << std::endl;
    };

    void operator() (const music& m) const {
      std::cout << "\nMuisc"
                << "\ntitle: " << m.title << "\nartist: " << m.artist << std::endl;
    };

    void operator() (const software& s) const {
      std::cout << "\nSoftware"
                << "\ntitle: " << s.title << "\nvendor: " << s.vendor << std::endl;
    };
  };

  for (const auto& d : dvds) {
    std::visit (visitor{}, d);
  }
}

/**
 * A visitor that does type matching that's implemented by providing a lambda
 * expression that performs an action based on the type of the alternative.
 */
inline void visit_with_map_matching_lambda_visitor (const Dvds& dvds) {
  auto visitor = [] (auto&& dvd) {
    // remove const volatile and reference
    using T = std::decay_t<decltype (dvd)>;

    // compile time check
    if constexpr (std::is_same_v<T, movie>) {
      std::cout << "\nMovie"
                << "\ntitle: " << dvd.title
                << "\nduration: " << dvd.duration.count () << " min" << std::endl;
    } else if constexpr (std::is_same_v<T, music>) {
      std::cout << "\nMuisc"
                << "\ntitle: " << dvd.title << "\nartist: " << dvd.artist << std::endl;
    } else if constexpr (std::is_same_v<T, software>) {
      std::cout << "\nSoftware"
                << "\ntitle: " << dvd.title << "\nvendor: " << dvd.vendor << std::endl;
    }
  };

  for (const auto& d : dvds) {
    std::visit (visitor, d);
  }
}

inline void test () {
  using namespace std::chrono_literals;
  const Dvds dvds{ movie{ "Movie", 115min }, music{ "Music", "Artist" },
    software{ "Software", "Vendor" } };
  visit_with_lambda (dvds);
  visit_with_visitor_that_returns (dvds);
  visit_with_map_matching_functor_visitor (dvds);
  visit_with_map_matching_lambda_visitor (dvds);
}
} // namespace visiting_and_variant
