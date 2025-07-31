
#include <CGAL/Polygon_2.h>
#include <CGAL/Simple_cartesian.h>

int main() {
  using K = CGAL::Simple_cartesian<float>;
  using Point_2 = K::Point_2;
  using Polygon_2 = CGAL::Polygon_2<K>;
  using Line_2 = CGAL::Line_2<K>;

  Point_2 p1{0, 0}, p2{1, 0}, p3{0, 1};
  auto triangle_aria = CGAL::area(p1, p2, p3);
  std::stringstream ss;
  ss << "triangle_aria:" << triangle_aria << "\n";
  std::cout << ss.str();

  Polygon_2 triangle;
  triangle.push_back(p1);
  triangle.push_back(p2);
  triangle.push_back(p3);
  ss.str("");
  ss << "triangle_aria:" << triangle.area() << "\n";
  std::cout << ss.str();

  Line_2 line{Point_2{0, 0}, Point_2{1, 1}};
  ss.str("");
  ss << "line from 2 points: " << line << "\n";
  ss << "point is on: " << line.has_on(Point_2{1, 1}) << "\n";
  ss << "point is on: " << line.has_on(Point_2{0, 1}) << "\n";
  ss << "point is on: " << line.has_on(Point_2{1, 0}) << "\n";
  ss << "point is on: " << line.has_on(Point_2{2, 2}) << "\n";
  std::cout << ss.str();

  Line_2 line1{-1, 1, 0};
  ss.str("");
  ss << "line from a, b, c: " << line1 << "\n";
  ss << "point is on: " << line1.has_on(Point_2{1, 1}) << "\n";
  ss << "point is on: " << line1.has_on(Point_2{0, 1}) << "\n";
  ss << "point is on: " << line1.has_on(Point_2{1, 0}) << "\n";
  ss << "point is on: " << line1.has_on(Point_2{2, 2}) << "\n";
  std::cout << ss.str();

  return 0;
}