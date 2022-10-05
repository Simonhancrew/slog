
#include "timezone.h"

#include <assert.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "date.h"
#include "noncopyable.h"
#include "types.h"
using std::vector;

namespace {

void EndianSwap(uint8_t* buf, int start, int length) {
  int end = start + length - 1;
  for (int i = 0, j = end; i < j; i++, j--) {
    std::swap(buf[i], buf[j]);
  }
}

struct Transition {
  time_t gmt_time_;
  time_t local_time_;
  int localtime_idx_;

  Transition(time_t t, time_t l, int localIdx)
      : gmt_time_(t), local_time_(l), localtime_idx_(localIdx) {
  }
};
struct Comp {
  bool compare_gmt_;

  Comp(bool gmt) : compare_gmt_(gmt) {
  }

  bool operator()(const Transition& lhs, const Transition& rhs) const {
    if (compare_gmt_)
      return lhs.gmt_time_ < rhs.gmt_time_;
    else
      return lhs.local_time_ < rhs.local_time_;
  }

  bool equal(const Transition& lhs, const Transition& rhs) const {
    if (compare_gmt_)
      return lhs.gmt_time_ == rhs.gmt_time_;
    else
      return lhs.local_time_ == rhs.local_time_;
  }
};

struct Localtime {
  time_t gmt_offset_;
  bool is_dst_;
  int arrb_idx_;

  Localtime(time_t offset, bool dst, int arrb)
      : gmt_offset_(offset), is_dst_(dst), arrb_idx_(arrb) {
  }
};

inline void FillHMS(unsigned seconds, struct tm* utc) {
  utc->tm_sec      = seconds % 60;
  unsigned minutes = seconds / 60;
  utc->tm_min      = minutes % 60;
  utc->tm_hour     = minutes / 60;
}

const uint32_t kSecondsPerDay = 24 * 60 * 60;
}  // namespace

namespace slog {

class File : Noncopyable {
 public:
  File(const char* file) : fp_(fopen(file, "rb")) {
  }

  ~File() {
    if (fp_) {
      fclose(fp_);
    }
  }

  bool Valid() const {
    return fp_;
  }

  std::string ReadBytes(int n) {
    vector<char> buf(n);
    int nr = fread(buf.data(), 1, n, fp_);
    if (nr != n)
      throw std::logic_error("no enough data");
    return std::string(buf.begin(), buf.end());
  }

  int32_t ReadInt32() {
    int32_t x = 0;
    int nr    = fread(&x, 1, sizeof(int32_t), fp_);
    if (nr != sizeof(int32_t))
      throw std::logic_error("bad int32_t data");
    EndianSwap(reinterpret_cast<uint8_t*>(&x), 0, sizeof x);
    return x;
  }

  uint8_t ReadUInt8() {
    uint8_t x = 0;
    int nr    = fread(&x, 1, sizeof(uint8_t), fp_);
    if (nr != sizeof(uint8_t))
      throw std::logic_error("bad uint8_t data");
    return x;
  }

 private:
  FILE* fp_;
};

struct TimeZone::Data {
  vector<Transition> transitions_;
  vector<Localtime> localtimes_;
  vector<std::string> names_;
  std::string abbreviation_;
};

bool ReadTimeZoneFile(const char* file, struct TimeZone::Data* data) {
  File f(file);
  if (f.Valid()) {
    try {
      // not sure how to implement on windows
    } catch (std::logic_error& e) {
      fprintf(stderr, "%s\n", e.what());
    }
  }
  return true;
}

const Localtime* FindLocaltime(const TimeZone::Data data, Transition sentry,
                               Comp comp) {
  const Localtime* local = nullptr;
  if (data.transitions_.empty() || comp(sentry, data.transitions_.front())) {
    // FIXME: should be first non dst time zone
    local = &data.localtimes_.front();
  } else {
    vector<Transition>::const_iterator trans_itor = std::lower_bound(
        data.transitions_.begin(), data.transitions_.end(), sentry, comp);
    if (trans_itor != data.transitions_.end()) {
      if (!comp.equal(sentry, *trans_itor)) {
        assert(trans_itor != data.transitions_.begin());
        --trans_itor;
      }
      local = &data.localtimes_[trans_itor->localtime_idx_];
    } else {
      // FIXME: use TZ-env
      local = &data.localtimes_[data.transitions_.back().localtime_idx_];
    }
  }
  return local;
}

TimeZone::TimeZone(const char* file) : data_(new TimeZone::Data) {
  if (!ReadTimeZoneFile(file, data_.get())) {
    data_.reset();
  }
}

TimeZone::TimeZone(int east_of_tc, const char* name)
    : data_(new TimeZone::Data) {
  data_->localtimes_.push_back(Localtime(east_of_tc, false, 0));
  data_->abbreviation_ = name;
}

struct tm TimeZone::ToLocalTime(time_t seconds) const {
  struct tm local_time;
  MemZero(&local_time, sizeof(local_time));
  assert(data_ != NULL);
  const Data& data(*data_);

  Transition sentry(seconds, 0, 0);
  const Localtime* local = FindLocaltime(data, sentry, Comp(true));

  if (local) {
    time_t local_seconds = seconds + local->gmt_offset_;
    local_time           = *gmtime(&local_seconds);  // FIXME: fromUtcTime
    local_time.tm_isdst  = local->is_dst_;
  }

  return local_time;
}

time_t TimeZone::FromLocalTime(const struct tm& local) const {
  assert(data_ != NULL);
  const Data& data(*data_);

  struct tm tmp  = local;
  time_t seconds = mktime(&tmp);  // FIXME: toUtcTime
  Transition sentry(0, seconds, 0);
  const Localtime* local_time = FindLocaltime(data, sentry, Comp(false));
  if (local.tm_isdst) {
    struct tm try_trans = ToLocalTime(seconds - local_time->gmt_offset_);
    if (!try_trans.tm_isdst && try_trans.tm_hour == local.tm_hour &&
        try_trans.tm_min == local.tm_min) {
      // FIXME: HACK
      seconds -= 3600;
    }
  }
  return seconds - local_time->gmt_offset_;
}

struct tm TimeZone::ToUtcTime(time_t seconds_since_epoch, bool yday) {
  struct tm utc;
  MemZero(&utc, sizeof(utc));
  // utc.tm_zone = "GMT";
  int seconds = static_cast<int>(seconds_since_epoch % kSecondsPerDay);
  int days    = static_cast<int>(seconds_since_epoch / kSecondsPerDay);
  if (seconds < 0) {
    seconds += kSecondsPerDay;
    --days;
  }
  FillHMS(seconds, &utc);
  Date date(days + Date::kJulianDayOf1970_01_01);
  Date::YearMonthDay ymd = date.GetDate();
  utc.tm_year            = ymd.year - 1900;
  utc.tm_mon             = ymd.month - 1;
  utc.tm_mday            = ymd.day;
  utc.tm_wday            = date.WeekDay();

  if (yday) {
    Date startOfYear(ymd.year, 1, 1);
    utc.tm_yday = date.JulianDayNumber() - startOfYear.JulianDayNumber();
  }
  return utc;
}

time_t TimeZone::FromUtcTime(const struct tm& utc) {
  return FromUtcTime(utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
                     utc.tm_hour, utc.tm_min, utc.tm_sec);
}

time_t TimeZone::FromUtcTime(int year, int month, int day, int hour, int minute,
                             int seconds) {
  Date date(year, month, day);
  const int seconds_in_day = hour * 3600 + minute * 60 + seconds;
  time_t days = date.JulianDayNumber() - Date::kJulianDayOf1970_01_01;
  return days * kSecondsPerDay + seconds_in_day;
}
}  // namespace slog
