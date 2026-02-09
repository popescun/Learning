//
// Created by Nicolae Popescu on 07/02/2026.
//

#pragma once

#include <iostream>
#include <set>
#include <unordered_map>

namespace isomorphic_strings {
using namespace std;

/**
 * Given two strings s and t, determine if they are isomorphic. Two strings s
 * and t are isomorphic if the characters in s can be replaced to get t.
 *
 * All occurrences of a character must be replaced with another character while
 * preserving the order of characters. No two characters may map to the same
 * character, but a character may map to itself.
 *
 * Examples:
 *  1. egg -> add
 *     e -> a
 *     g -> d
 *     g -> d
 *
 *  No mapping is changed , the strings are isomorphic.
 *
 *  2. f11 -> b23
 *    f -> b
 *    1 -> 2
 *    1 -> 3 // Existing key  is remapped to new value
 *
 *    Existing key (1) is remapped, the strings are not isomorphic.
 *
 *    We need a map m to store the mapping.
 *    If m contains key and the value for key !=  value(char from string)
 *      then the strings are not isomorphic.
 *
 *  3. badc -> baba
 *    b -> b
 *    a -> a
 *    d -> b // Existing value b is remapped to new key d
 *    c -> a // Existing value a is remapped to new key c
 *
 *    Existing values (b, a) are re-mapped, the string are not isomorphic.
 *
 *    If m contains value and the key for value != key(char from string)
 *      then the strings are not isomorphic.
 *
 *    To find the key for value is costly, but this is equivalent to check if
 * next key exists in the mapping. If it does not exist then it is different
 * from whatever was mapped to the value.
 *
 *    If m contains value and m does not contain next key
 *      then the strings are not isomorphic.
 */

inline bool isIsomorphic(string s, string t) {
  if (s.size() != t.size() || s == t) {
    return false;
  }

  std::unordered_map<char, char> m;
  std::set<char> values;

  for (size_t i = 0; i < s.size(); ++i) {
    // check existing key is re-mapped
    if ((m.contains(s[i]) && m[s[i]] != t[i]) ||
        // check existing value is re-mapped
        (values.contains(t[i]) && !m.contains(s[i]))) {
      return false;
    }
    m[s[i]] = t[i];
    values.insert(t[i]);
  }
  return true;
}

inline void test() {
  auto s = "egg";
  auto t = "add";
  auto print = [&]() {
    std::cout << s << " and " << t << " are"
              << (isIsomorphic(s, t) ? "" : " not") << " isomorphic"
              << std::endl;
  };
  print();
  s = "f11";
  t = "b23";
  print();
  s = "paper";
  t = "title";
  print();
  s = "badc";
  t = "baba";
  print();
  s = "bbbaaaba";
  t = "aaabbbba";
  print();
  s = "papap";
  t = "titii";
  print();
  s = "abab";
  t = "baba";
  print();
  s = "qwertyuiop[]asdfghjkl;'\\zxcvbnm,./";
  t = "',.pyfgcrl/=aoeuidhtns-\\;qjkxbmwvz";
  print();
}
} // namespace isomorphic_strings