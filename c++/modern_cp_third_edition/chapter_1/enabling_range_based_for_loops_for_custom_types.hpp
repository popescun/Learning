#include <cstddef>
#include <stdexcept>
#include <iostream>

namespace enabling_range_based_for_loops_for_custom_types {
  template<typename T, std::size_t const Size>
  class dummy_array {
  public:
    [[nodiscard]] const auto& get_at(std::size_t const index) const {
      if (index < Size) {
        return data_[index];
      } else {
        throw std::out_of_range("index is out of range");
      }
    }

    void set_at(std::size_t const index, const T &value) {
      if (index < Size) {
        data_[index] = value;
      } else {
        throw std::out_of_range("index is out of range");
      }
    }

    [[nodiscard]] auto get_size() const {
      return Size;
    }

  private:
    T data_[Size] = {};
  };

  void test() {
    dummy_array<int, 3> arr;
    arr.set_at(0, 1);
    arr.set_at(1, 2);
    arr.set_at(2, 3);
    for(std::size_t i = 0; i < arr.get_size(); ++i) {
      std::cout << arr.get_at(i) << std::endl;
    }
//    for(const auto& item : arr) {
//      std::cout << item << std::endl;
//    }
  }
}


