#include <print>

namespace test_dome {
class MegaStore {
public:
  enum class DiscountType { Standard, Seasonal, Weight };

  static double getDiscountedPrice(double cartWeight, double totalPrice,
                                   DiscountType discountType) {
    switch (discountType) {
    case DiscountType::Standard:
      return (100 - 6) * totalPrice / 100;
    case DiscountType::Seasonal:
      return (100 - 12) * totalPrice / 100;
    case DiscountType::Weight:
      double percent = 0;
      percent = cartWeight <= 10 ? 100 - 6 : 100 - 18;
      return percent * totalPrice / 100;
    }
    return 0.0;
  }
};

inline void test() {
  // std::println(
      // "discount price:{}",
      // MegaStore::getDiscountedPrice(12, 100, MegaStore::DiscountType::Weight));
}
} // namespace test_dome