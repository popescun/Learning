//
// Created by Nicolae Popescu on 23/07/2025.
//

#pragma once

#include <map>
#include <sstream>
#include <tuple>

namespace using_structured_bindings_to_handle_multi_return_values {

std::tuple<int, std::string, double> find() {
  return std::make_tuple(1, "nicu", 1234.5);
}

inline void test() {
  auto [id, name, score] = find();
  std::stringstream ss;
  ss << "id:" << id << ", name:" << name << ", score:" << score;
  std::cout << ss.str() << std::endl;

  if (auto [id1, name1, score1] = find(); score1 > 1000) {
    std::cout << name << std::endl;
  }

  // the result of map::insert is not easy to read
  std::map<int, std::string> m;
  auto result = m.insert({1, "one"});
  ss.str("");
  ss << "inserted:" << result.second << ", value:" << result.first->second
     << "\n";
  std::cout << ss.str();

  // make it more readable with std::tie
  std::map<int, std::string>::iterator it;
  bool inserted;
  std::tie(it, inserted) = m.insert({2, "two"});
  ss.str("");
  ss << "inserted:" << inserted << ", value:" << it->second << "\n";
  std::cout << ss.str();

  std::tie(it, inserted) = m.insert({2, "two"});
  ss.str("");
  ss << "inserted:" << inserted << ", value:" << it->second << "\n";
  std::cout << ss.str();

  // c++ 17 structured binding
  {
    auto [it, inserted] = m.insert({3, "three"});
    ss.str("");
    ss << "inserted:" << inserted << ", value:" << it->second << "\n";
    std::cout << ss.str();
  }

  {
    auto [it, inserted] = m.insert({3, "three"});
    ss.str("");
    ss << "inserted:" << inserted << ", value:" << it->second << "\n";
    std::cout << ss.str();
  }

  {
    auto [it, inserted] = m.insert({4, "four"});
    ss.str("");
    ss << "inserted:" << inserted << ", value:" << it->second << "\n";
    std::cout << ss.str();
  }

  if (auto [it, inserted] = m.insert({5, "five"}); inserted) {
    ss.str("");
    ss << "value:" << it->second << "\n";
    std::cout << ss.str();
  }
}
} // namespace using_structured_bindings_to_handle_multi_return_values
