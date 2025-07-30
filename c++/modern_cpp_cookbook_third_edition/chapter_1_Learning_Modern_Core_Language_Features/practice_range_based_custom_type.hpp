//
// Created by Nicolae Popescu on 30/07/2025.
//

#ifndef PRACTICE_RANGE_BASED_CUSTOM_TYPE_HPP
#define PRACTICE_RANGE_BASED_CUSTOM_TYPE_HPP

#include <exception>
#include <iostream>

namespace practice_range_based_custom_type {

  // we need to implement == or !=, * and ++ class operators
  template<typename T, typename C>
  class custom_array_iterator final {
  public:
    custom_array_iterator(C& collection, std::size_t const index)
      : collection_{collection},
        index_{index} {
    }

    bool operator==(const auto& other) const {
      return index_ == other.index_;
    }

    const T& operator*() const {
      return collection_.get_at(index_);
    }

    auto& operator++() {
      ++index_;
      return *this;
    }

  private:
    C& collection_;
    std::size_t index_{0};
  };

  // we need to provide class or global begin() and end() operations
  template<typename T, std::size_t Size>
  class custom_array final {
  public:
    custom_array(const std::initializer_list<T>& data) {
      // copy data using a pointer
//    auto temp = data_;
//    for(const auto& element : data) {
//      *temp++ = element;
//    }
      std::copy(data.begin(), data.end(), data_);
    }

    // explicit is required as variadic means also 1 argument
    template<typename...  args>
    explicit custom_array(args... data) : data_{data...} {

    }

    void set_at(std::size_t index, T value) {
      if( index >= Size) {
        throw std::runtime_error("index is out of range");
      }
      data_[index] = value;
    }

    [[nodiscard]] const T& get_at(std::size_t const index) const {
      if( index >= Size) {
        throw std::runtime_error("index is out of range");
      }
      return data_[index];
    }

    /* Interestingly if both constructors above are provided,
     * the compiler picks up the initializer list one if the object
     * is constructed with initializer list {}. Though the variadic
     * templated constructor works with standard initialization () for
     * any numbers of arguments.
     */

    [[nodiscard]] std::size_t size() const {
      return Size;
    }

    auto begin() {
      return custom_array_iterator<T, custom_array<T, Size>>{*this, 0};
    }

    auto end() {
      return custom_array_iterator<T, custom_array<T, Size>>{*this, size()};
    }

  private:
    T data_[Size] = {};
  };

  inline void test() {
    custom_array<int, 3> arr{10, 20, 30};
    arr.set_at(2, 40);
//    custom_array<int, 3> arr1(1, 2, 3);

    std::stringstream ss;
    for (const auto& element : arr) {
      ss << element << " ";
    }
    std::cout << ss.str() << std::endl;
  }
}

#endif // PRACTICE_RANGE_BASED_CUSTOM_TYPE_HPP
