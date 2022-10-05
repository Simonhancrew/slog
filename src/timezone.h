#pragma once
#include <ctime>
#include <memory>

#include "copyable.h"

namespace slog {

class TimeZone : public Copyable {
 public:
  explicit TimeZone(const char* zonefile);
  TimeZone(int east_of_utc, const char* tzname);  // a fixed timezone
  TimeZone() = default;                           // an invalid timezone

  // default copy ctor/assignment/dtor are Okay.

  bool Valid() const {
    // 'explicit operator bool() const' in C++11
    return static_cast<bool>(data_);
  }

  struct tm ToLocalTime(time_t secondsSinceEpoch) const;
  time_t FromLocalTime(const struct tm&) const;

  // gmtime(3)
  static struct tm ToUtcTime(time_t seconds_since_epoch, bool yday = false);
  // timegm(3)
  static time_t FromUtcTime(const struct tm&);
  // year in [1900..2500], month in [1..12], day in [1..31]
  static time_t FromUtcTime(int year, int month, int day, int hour, int minute,
                            int seconds);

  struct Data;

 private:
  std::shared_ptr<Data> data_;
};

}  // namespace slog
