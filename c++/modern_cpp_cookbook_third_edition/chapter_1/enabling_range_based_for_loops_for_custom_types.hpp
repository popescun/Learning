#pragma once

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

  template<typename T, typename C, std::size_t const Size>
  class dummy_array_iterator_type {
  public:
    dummy_array_iterator_type(C& collection, std::size_t const index)
    : index_{index},
      collection_{collection} {
    }

    // one of the operators == or !=
    bool operator==(dummy_array_iterator_type const & other) const {
      return index_ == other.index_;
    }

    T const& operator*() const {
      return collection_.get_at(index_);
    }

    dummy_array_iterator_type& operator++() {
      index_++;
      return *this;
    }

    // not really required for range-based loop
    // dummy_array_iterator_type operator++(int) {
    //   auto temp = *this;
    //   ++*temp;
    //   return temp;
    // }

  private:
    std::size_t index_;
    C& collection_;
  };

  template<typename T, std::size_t const Size>
  using dummy_array_iterator =
    dummy_array_iterator_type<T, dummy_array<T, Size>, Size>;

  template<typename T, std::size_t const Size>
  using dummy_array_const_iterator =
    dummy_array_iterator_type<T, dummy_array<T, Size> const, Size>;

  template<typename T, std::size_t const Size>
  dummy_array_iterator<T, Size> begin(dummy_array<T, Size>& collection) {
    return dummy_array_iterator<T, Size>(collection, 0);
  }

  template<typename T, std::size_t const Size>
  dummy_array_iterator<T, Size> end(dummy_array<T, Size>& collection) {
    return dummy_array_iterator<T, Size>(collection, collection.get_size());
  }

  // when const versions for begin and end are required?
  // template<typename T, std::size_t const Size>
  // dummy_array_const_iterator<T, Size> begin(dummy_array<T, Size> const& collection) {
  //   return dummy_array_const_iterator<T, Size>(collection, 0);
  // }
  //
  // template<typename T, std::size_t const Size>
  // dummy_array_const_iterator<T, Size> end(dummy_array<T, Size> const& collection) {
  //   return dummy_array_const_iterator<T, Size>(collection, collection.get_size());
  // }

  inline void test() {
    dummy_array<int, 3> arr;
    arr.set_at(0, 1);
    arr.set_at(1, 2);
    arr.set_at(2, 3);

    for(std::size_t i = 0; i < arr.get_size(); ++i) {
      std::cout << arr.get_at(i) << std::endl;
    }
    for(const auto& item : arr) {
      std::cout << item << std::endl;
    }
  }
}


