#include <iostream>
#include <vector>

namespace eurocontrol {
  int usageCount(const std::vector<int> &bridge){
    const auto min_step = std::ranges::min_element(bridge);
    return *min_step / 2;
  }

  class ChristmasLights
  {
  private:
    const int lightCount;

  public:
    ChristmasLights(int lightCount) : lightCount(lightCount) {}

    std::vector<bool> next()
    {
      static int next = 1;

      std::vector<bool> lights(lightCount, false);
      lights[0] = true;
      lights[lightCount - 1] = true;
      if (right_direction) {
        lights[next++] = true;
        if (next == lightCount - 2) {
          right_direction = false;
        }
      } else {
        lights[next--] = true;
        if (next == 1) {
          right_direction = true;
        }
      }


      return lights;
    }
  private:
    bool right_direction = true;
  };

  bool simulate(bool input1, bool input2, bool input3, bool input4)
  {
    bool gate1 = !input1;
    bool gate2 = gate1 || input2;
    bool gate3 = gate2 && input3;
    bool gate4 = input2 || input3;
    bool gate5 = gate3 || input4;
    bool gate6 = !gate4;
    bool gate7 = gate5 && gate6;
    return gate7;
  }

inline void test() {
    std::vector<int> bridge = {7, 6, 5, 8};
    std::cout << usageCount(bridge) << std::endl; // Should print 2

  ChristmasLights lights(9);

  // for (int i = 0; i < 11; i++)
  // {
  //   std::vector<bool> result = lights.next();
  //   std::cout << "Step " << i + 1 << ": ";
  //   std::cout << std::boolalpha;
  //   for (bool light : result)
  //   {
  //     std::cout << light << ", ";
  //   }
  //   std::cout << std::endl;
  // }

  // std::cout << simulate(false, false, false, true) << std::endl; // should print 1
}
} // namespace eurocontrol