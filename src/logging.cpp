#include "logging.h"

#include <string.h>

#include <thread>

namespace {

const uint32_t kMaxTypes = static_cast<uint32_t>(slog::LogLevel::kMaxTypes);
char error_buf[512];
char time_buf[64];
time_t g_last_second;
slog::TimeZone g_time_zone{};
const char* StrError(int err_num) {
  strerror_s(error_buf, sizeof error_buf, err_num);
  return error_buf;
}
}  // namespace

namespace slog {

LogLevel global_level = Logger::GLogLevel();

const char* LevelName[kMaxTypes] = {"INFO  ", "DEBUG ", "WARRN ", "ERROR ",
                                    "FATAL "};
// compile time calcu string len
class T {
 public:
  T(const char* str, unsigned len) : str_(str), len_(len) {
    assert(strlen(str) == len_);
  }

  const char* str_;
  const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v) {
  s.Append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const slog::SourceFile& v) {
  s.Append(v.data_, v.size_);
  return s;
}

void DefaultOutput(const char* msg, int len) {
  size_t n = fwrite(msg, 1, len, stdout);
  // FIXME check n
  (void)n;
}

void DefaultFlush() {
  fflush(stdout);
}

Logger::OutputFuc GOoutPutFunc = DefaultOutput;
Logger::FlushFunc GFlushFunc   = DefaultFlush;

Impl::Impl(LogLevel level, int err_no, const SourceFile& file, int line)
    : time_(Timestamp::Now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
  FormatTime();
  stream_ << T(LevelName[static_cast<int>(level)], 6);
  if (err_no != 0) {
    stream_ << StrError(errno) << " (errno=" << err_no << ")";
  }
}

void Impl::FormatTime() {
  int64_t now_ms = time_.MicroSecondsSinceEpoch();
  time_t seconds =
      static_cast<time_t>(now_ms / Timestamp::kMicroSecondsPerSecond);
  int ms = static_cast<int>(now_ms % Timestamp::kMicroSecondsPerSecond);
  if (seconds != g_last_second) {
    g_last_second = seconds;
    struct tm time_tm;
    if (g_time_zone.Valid()) {
      time_tm = g_time_zone.ToLocalTime(seconds);
    } else {
      localtime_s(&time_tm, &seconds);
    }
    int len =
        snprintf(time_buf, sizeof(time_buf), "%4d%02d%02d %02d:%02d:%02d",
                 time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday,
                 time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec);
    assert(len == 17);
    (void)len;
  }

  if (g_time_zone.Valid()) {
    Fmt us(".%06d ", ms);
    assert(us.Length() == 8);
    stream_ << T(time_buf, 17) << T(us.Data(), 8);
  } else {
    Fmt us(".%06dZ ", ms);
    assert(us.Length() == 9);
    stream_ << T(time_buf, 17) << T(us.Data(), 9);
  }
}

void Impl::Finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
    : impl_(LogLevel::kInfo, 0, file, line) {
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {
}

Logger::Logger(SourceFile file, int line, bool to_abort)
    : impl_(to_abort ? LogLevel::kFatal : LogLevel::kError, errno, file, line) {
}

Logger::~Logger() {
  impl_.Finish();
  const LogStream::Buffer& buf(Stream().buffer());
  GOoutPutFunc(buf.Data(), buf.Length());
  if (impl_.level_ == LogLevel::kFatal) {
    GFlushFunc();
    abort();
  }
}

void Logger::SetLogLevel(LogLevel level) {
  global_level = level;
}

// FIX:move
void Logger::SetFlush(FlushFunc func) {
  GFlushFunc = func;
}

void Logger::SetOutput(OutputFuc func) {
  GOoutPutFunc = func;
}

void Logger::SetTimeZone(const TimeZone& tz) {
  g_time_zone = tz;
}

}  // namespace slog
