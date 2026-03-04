#include <iostream>

/**
 * Unsigned integer arithmetic is always performed modulo 2^n where n is the number 
 * of bits in that particular integer. E.g. for unsigned int, adding one to UINT_MAX 
 * gives ​0​, and subtracting one from ​0​ gives UINT_MAX.
 */

namespace integers_overflow {
inline void test() {
  auto a {UINT32_MAX};
  unsigned int b {0};
  std::cout << "UINT32_MAX:" << a << std::endl;
  std::cout << "UINT32_MAX + 1:" << a + 1 << std::endl;
  std::cout << "unsigned int 0 - 1:" << b - 1 << std::endl;
}    
}