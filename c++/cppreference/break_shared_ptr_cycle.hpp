#include <memory>
#include <print>

namespace break_shared_ptr_cycle {

struct Dog; // Forward declaration

struct Owner {
  std::string name;
  std::weak_ptr<Dog> my_dog; // Weak pointer to Dog
  ~Owner() {
    // std::println("Owner destroyed");
  }
};

struct Dog {
  std::string name;
  std::shared_ptr<Owner> my_owner; // Strong pointer to Owner
  ~Dog() {
    // std::println("Dog destroyed");
  }
};

inline void test() {

  auto person = std::make_shared<Owner>();
  auto pup = std::make_shared<Dog>();
  person->my_dog = pup;   // pup ref count = 1
  pup->my_owner = person; // person ref count = 2
  // std::println("person ref count:{}", person.use_count());
  // std::println("pup ref count:{}", pup.use_count());
}
} // namespace break_shared_ptr_cycle