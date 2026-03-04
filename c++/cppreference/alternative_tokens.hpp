#include <iostream>

namespace alternative_tokens {
inline void test() {
  int a[10];

  a[0] = 22;

  std::cout << a[0] << std::endl;
  std::cout << a<:0:> << std::endl;
}    
}