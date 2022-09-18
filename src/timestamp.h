#pragma once
#include <algorithm>
#include <cstdint>
#include <string>

#include "copyable.h"

namespace slog {

//
// Time stamp in UTC, in microseconds resolution.
//
// This class is immutable.
// It's recommended to pass it by value, since it's passed in register on x64.

class Timestamp : public copyable {
 public:
  //
  // Constucts an invalid Timestamp.
  //
  Timestamp() : microseconds_since_epoch_(0) {
  }

  //
  // Constucts a Timestamp at specific time
  //
  // @param microSecondsSinceEpoch
  explicit Timestamp(int64_t microseconds_since_epoch_arg)
      : microseconds_since_epoch_(microseconds_since_epoch_arg) {
  }

  void Swap(Timestamp &that) {
    std::swap(microseconds_since_epoch_, that.microseconds_since_epoch_);
  }

  // default copy/assignment/dtor are Okay

  std::string ToString() const;
  std::string ToFormattedString(bool show_microseconds = true) const;

  bool Valid() const {
    return microseconds_since_epoch_ > 0;
  }

  // for internal usage.
  int64_t MicroSecondsSinceEpoch() const {
    return microseconds_since_epoch_;
  }
  time_t SecondsSinceEpoch() const {
    return static_cast<time_t>(microseconds_since_epoch_ /
                               kMicroSecondsPerSecond);
  }

  //
  // Get time of now.
  //
  static Timestamp Now();
  static Timestamp Invalid() {
    return Timestamp();
  }

  static Timestamp FromUnixTime(time_t t) {
    return FromUnixTime(t, 0);
  }

  static Timestamp FromUnixTime(time_t t, int microseconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond +
                     microseconds);
  }

  static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  int64_t microseconds_since_epoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.MicroSecondsSinceEpoch() < rhs.MicroSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.MicroSecondsSinceEpoch() == rhs.MicroSecondsSinceEpoch();
}

//
// Gets time difference of two timestamps, result in seconds.
//
// @param high, low
// @return (high-low) in seconds
// @c double has 52-bit precision, enough for one-microsecond
// resolution for next 100 years.
inline double TimeDifference(Timestamp high, Timestamp low) {
  int64_t diff = high.MicroSecondsSinceEpoch() - low.MicroSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

//
// Add @c seconds to given timestamp.
//
// @return timestamp+seconds as Timestamp
//
inline Timestamp AddTime(Timestamp timestamp, double seconds) {
  int64_t delta =
      static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.MicroSecondsSinceEpoch() + delta);
}

}  // namespace slog
