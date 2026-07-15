#include <print>

namespace test_alignas {

struct S {
// struct alignas(2) S {
  // calculate size:
  //  - determine the minimum alignment which is the size of the biggest field
  //  - determine min alignments needed to store the fields in the order of declaration
  //  e.g.
  //       struct S {
  //          int8_t z = 0;
  //          int16_t y = 0;
  //          int32_t x = 0;}
  //    min alignment is sizeof(int32) = 4 bytes
  //    z and y fits in 4 bytes + 4 bytes for x = 8 bytes
  //
  int16_t y = 0;
  int32_t x = 0;
  int8_t z = 0;
};

inline void test() {
  S s;
  // std::println("S size:{}", sizeof(S));
  // std::println("s size:{}", sizeof(s));
}
} // namespace test_alignas