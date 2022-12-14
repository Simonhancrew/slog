
#include "timestamp.h"

#include <inttypes.h>

#include "stdio.h"
#include "time.h"
#include "time_utils_base.h"

namespace slog {

std::string Timestamp::ToString() const {
  char buf[32]         = {0};
  int64_t seconds      = microseconds_since_epoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microseconds_since_epoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds,
           microseconds);
  return buf;
}

std::string Timestamp::ToFormattedString(bool show_microseconds) const {
  char buf[64] = {0};
  time_t seconds =
      static_cast<time_t>(microseconds_since_epoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_s(&tm_time, &seconds);

  if (show_microseconds) {
    int microseconds =
        static_cast<int>(microseconds_since_epoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

Timestamp Timestamp::Now() {
  return Timestamp(NowMs());
}

}  // namespace slog
