//
// Created by Nicolae Popescu on 10/11/2025.
//

#pragma once

#include <mach/mach_time.h>
#include <time.h>

namespace utils {
struct measure_matched_time {
  std::uint64_t start_{0};
  std::uint64_t time_{0};
  std::uint64_t accumulator_{0};

  measure_matched_time() { start(); }

  void start() { start_ = mach_absolute_time(); }
  void stop() {
    time_ = mach_absolute_time() - start_;
    accumulator_ += time_;
  }

  [[nodiscard]] auto get_time_ns() const {
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return static_cast<double>(time_) * static_cast<double>(info.numer) /
           info.denom;
  }

  [[nodiscard]] auto get_accumulated_time_ns() const {
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return static_cast<double>(accumulator_) * static_cast<double>(info.numer) /
           info.denom;
  }

  [[nodiscard]] auto get_time_us() const { return get_time_ns() / 1000; }

  [[nodiscard]] auto get_accumulated_time_us() const {
    return get_accumulated_time_ns() / 1000;
  }
};

struct measure_time {
  std::uint64_t start_{0};
  std::uint64_t time_{0};
  std::uint64_t accumulator_{0};

  measure_time() { start(); }

  void start() { start_ = clock_gettime_nsec_np(_CLOCK_MONOTONIC_RAW); }
  void stop() {
    time_ = clock_gettime_nsec_np(_CLOCK_MONOTONIC_RAW) - start_;
    accumulator_ += time_;
  }

  [[nodiscard]] auto get_time_ns() const { return time_; }

  [[nodiscard]] auto get_accumulated_time_ns() const { return accumulator_; }

  [[nodiscard]] auto get_time_us() const { return get_time_ns() / 1000; }

  [[nodiscard]] auto get_accumulated_time_us() const {
    return get_accumulated_time_ns() / 1000;
  }
};
} // namespace utils