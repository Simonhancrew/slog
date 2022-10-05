#pragma once
#include "copyable.h"
#include "types.h"

struct tm;

namespace slog {

class Date : public Copyable {
 public:
  struct YearMonthDay {
    int year;   // [1900..2500]
    int month;  // [1..12]
    int day;    // [1..31]
  };

  static const int kDaysPerWeek = 7;
  static const int kJulianDayOf1970_01_01;

  ///
  /// Constucts an invalid Date.
  ///
  Date() : julian_day_number_(0) {
  }

  ///
  /// Constucts a yyyy-mm-dd Date.
  ///
  /// 1 <= month <= 12
  Date(int year, int month, int day);

  ///
  /// Constucts a Date from Julian Day Number.
  ///
  explicit Date(int julianDayNum) : julian_day_number_(julianDayNum) {
  }

  ///
  /// Constucts a Date from struct tm
  ///
  explicit Date(const struct tm&);

  // default copy/assignment/dtor are Okay

  void Swap(Date& that) {
    std::swap(julian_day_number_, that.julian_day_number_);
  }

  bool Valid() const {
    return julian_day_number_ > 0;
  }

  ///
  /// Converts to yyyy-mm-dd format.
  ///
  string ToIsoString() const;

  struct YearMonthDay GetDate() const;

  int Year() const {
    return GetDate().year;
  }

  int Month() const {
    return GetDate().month;
  }

  int Day() const {
    return GetDate().day;
  }

  // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
  int WeekDay() const {
    return (julian_day_number_ + 1) % kDaysPerWeek;
  }

  int JulianDayNumber() const {
    return julian_day_number_;
  }

 private:
  int julian_day_number_;
};

inline bool operator<(Date x, Date y) {
  return x.JulianDayNumber() < y.JulianDayNumber();
}

inline bool operator==(Date x, Date y) {
  return x.JulianDayNumber() == y.JulianDayNumber();
}
}  // namespace slog
