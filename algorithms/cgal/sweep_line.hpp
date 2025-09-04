//
// Created by Nicolae Popescu on 04/09/2025.
//

#ifndef CGAL_SWEEP_LINE_H
#define CGAL_SWEEP_LINE_H

// this header seems needed explicitly, to prevent error like:
// error: incomplete type 'boost::multiprecision::backends::gmp_int' used in type trait expression
#include <boost/multiprecision/gmp.hpp>

#include <list>

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Surface_sweep_2_algorithms.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits_2;
typedef Traits_2::Curve_2 Segment_2;
typedef std::list<Segment_2> SegmentList;

namespace sweep_line {
inline void test() {
  SegmentList segments;
  Segment_2 s1(Point_2(1, 5), Point_2(8, 5));
  Segment_2 s2(Point_2(1, 1), Point_2(8, 8));
  Segment_2 s3(Point_2(3, 1), Point_2(3, 8));
  Segment_2 s4(Point_2(8, 5), Point_2(8, 8));

  segments.push_back(s1);
  segments.push_back(s2);
  segments.push_back(s3);
  segments.push_back(s4);

  std::list<Point_2> intersections;
  CGAL::compute_intersection_points(segments.begin(), segments.end(),
                                    std::back_inserter(intersections));
  std::cout << "Found " << intersections.size()
            << " intersections: " << std::endl;
  std::copy(intersections.begin(), intersections.end(),
            std::ostream_iterator<Point_2>(std::cout, "\n"));
}

} // namespace sweep_line

#endif // CGAL_SWEEP_LINE_H