//
// Created by Nicolae Popescu on 29/07/2025.
//

#ifndef CHAPTER_20_GEOMETRIC_PRIMTIVES_HPP
#define CHAPTER_20_GEOMETRIC_PRIMTIVES_HPP

#include <iostream>
#include <sstream>

/*
 Good explanation why is more consistent to write `const` specifier after type:
 this way, everything to the left of `const` is meant to be const.

 http://www.mario-konrad.ch/blog/programming/cpp-style-const.html
 */

namespace geometric_primitives {
struct Point {
  float x;
  float y;
};

struct Triangle {
  Point p1;
  Point p2;
  Point p3;
};

/* Compute a 2d determinant
 |p1.x p1.y 1|
 |p2.x p2.y 1| = p1.x * p2.y + p1.y * p3.x + p2.x * p3.y - p3.x * p2.y - p2.x *
 p1.y - p1.x * p3.y |p3.x p3.y 1|
 */

float determinant_2D(const Point p1, const Point p2, const Point p3) {
  return p1.x * p2.y + p1.y * p3.x + p2.x * p3.y - p3.x * p2.y - p2.x * p1.y -
         p1.x * p3.y;
}

/*
 2 * A(t) = D(t), where t - triangle
 */
float aria_of_triangle(const Point p1, const Point p2, const Point p3) {
  return std::abs(determinant_2D(p1, p2, p3) / 2);
}

float aria_of_triangle(const Triangle t) {
  return aria_of_triangle(t.p1, t.p2, t.p3);
}

/*
 Above formula generalize to:
 d! * V(p) = D(p), where d - dimensions
                         p - polyhedron
                         V - volume (in 2D is an aria)
                         D - determinant
 */

/* Compute where a point is opposite a line?

 Given a line equation: y  = ax + b and a point p, compute if p lies
 above, below, or on the line?
 */

enum class Relation {
  Above,
  Bellow,
  Overlap
};

Relation point_and_line_relation() {
  return Relation::Overlap;
}

inline void test() {
  auto triangle_aria = aria_of_triangle({0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0});
  std::stringstream ss;
  ss << "triangle_aria:" << triangle_aria << "\n";
  std::cout << ss.str();

  triangle_aria = aria_of_triangle({{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}});
  ss.str("");
  ss << "triangle_aria:" << triangle_aria << "\n";
  std::cout << ss.str();
}
} // namespace geometric_primitives

#endif // CHAPTER_20_GEOMETRIC_PRIMTIVES_HPP
