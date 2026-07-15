#include <memory>
#include <print>
#include <string>
#include <type_traits>

namespace misc {

constexpr int square_constexpr(int x) { return x * x; }

consteval int square_consteval(int x) { return x * x; }

inline void test() {

  auto ret = std::is_same<int, int>().value;
  // std::println("std::is_same<int, int>::value:{}",
  //              std::is_same<int, int>::value);
  static_assert(std::is_same<int, int>::value);

  std::allocator<int> alloc1;
  static_assert(std::is_same_v<int, decltype(alloc1)::value_type>);

  int *p1 = alloc1.allocate(1);
  alloc1.deallocate(p1, 1);

  int *p2 = new int[10]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  ::new (p2) int(
      5); // int has trivial destructor that is called for original p[0]
  for (int i = 0; i < 10; ++i) {
    // std::println("p[{}]:{}", i, p2[i]);
  }
  delete[] p2;

  std::string *p3 = new std::string[10]{"a", "b", /* ... */};
  // p3[0].std::string::~string();
  // p3[0].std::string::~basic_string(); // preferred dtor,a s std::string is
  // just and alias to std::basic_string
  std::destroy_at(p3 + 0);         // modern way of destroying an object
  ::new (p3) std::string("hello"); // ← LEAK, string destructor is not trivial
                                   // and is not called here by default;
  delete[] p3;

  constexpr auto n = 10;
  if (true) {
    constexpr int sq1 = square_constexpr(n);
    // std::println("sq:{}", sq1);
    constexpr int sq2 = square_consteval(n);
    // std::println("sq:{}", sq2);
  }

  /**
    A useful rule of thumb:
    constexpr = can be evaluated at compile time.
    consteval = must be evaluated at compile time.
   */

  int n1 = 5;
  // here constexpr on function is ignored as `a` is a run time variable
  int a = square_constexpr(n1); // OK: runtime call
  // here constexpr on variable requires everything to be const at compile time
  // constexpr int b = square_constexpr(n1); // error: constexpr variable 'b'
  // must be initialized by a constant expression note: read of non-const
  // variable 'n1' is not allowed in a constant expression
  // here consteval on function enforce that n1 to be const at compile time
  // int c = square_consteval(n1); // error: call to consteval function
  // 'misc::square_consteval' is not a constant expression note: read of
  // non-const variable 'n1' is not allowed in a constant expression
  // constexpr on variable requires d to be init by compile time const
  // expression, which cannot be evaluated from function as it gets a non const
  // argument constexpr int d = square_consteval(n1); // error: constexpr
  // variable 'd' must be initialized by a constant expression
} // note: read of non-const variable 'n1' is not allowed in a constant
  // expression
} // namespace misc