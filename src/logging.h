#pragma once
#include "log_stream.h"
#include "timestamp.h"
#include "timezone.h"

namespace slog {

enum class LogLevel {
  kInfo,
  kDebug,
  kWarrn,
  kError,
  kFatal,
  kMaxTypes,
};

// compile time calculation of basename of source file
class SourceFile {
 public:
  template<int N>
  SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
    const char* slash = strrchr(data_, '/');  // builtin function
    if (slash) {
      data_ = slash + 1;
      size_ -= static_cast<int>(data_ - arr);
    }
  }

  explicit SourceFile(const char* filename) : data_(filename) {
    const char* slash = strrchr(filename, '/');
    if (slash) {
      data_ = slash + 1;
    }
    size_ = static_cast<int>(strlen(data_));
  }

  const char* data_;
  int size_;
};

class Impl {
 public:
  Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
  void FormatTime();
  void Finish();

  Timestamp time_;
  LogStream stream_;
  LogLevel level_;
  int line_;
  SourceFile basename_;
};

class Logger {
 public:
  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool to_abort);
  ~Logger();

  LogStream& Stream() {
    return impl_.stream_;
  }

  static slog::LogLevel GLogLevel();
  static void SetLogLevel(slog::LogLevel level);

  using OutputFuc = void (*)(const char* msg, int len);
  using FlushFunc = void (*)();

  static void SetOutput(OutputFuc);
  static void SetFlush(FlushFunc);
  static void SetTimeZone(const TimeZone& tz);

 private:
  Impl impl_;
};

extern LogLevel global_level;

inline LogLevel Logger::GLogLevel() {
  return global_level;
}

#define LOG_DEBUG                                    \
  if (slog::Logger::GLogLevel() <= LogLevel::kDebug) \
  slog::Logger(__FILE__, __LINE__, LogLevel::DEBUG, __FUNCTION__).Stream()
#define LOG_INFO                                    \
  if (slog::Logger::GLogLevel() <= LogLevel::kInfo) \
  slog::Logger(__FILE__, __LINE__).Stream()
#define LOG_WARN \
  slog::Logger(__FILE__, __LINE__, slog::LogLevel::kWarrn).Stream()
#define LOG_ERROR \
  slog::Logger(__FILE__, __LINE__, slog::LogLevel::kError).Stream()
#define LOG_FATAL \
  slog::Logger(__FILE__, __LINE__, slog::LogLevel::kFatal).Stream()

#define CHECK_NOTNULL(val) \
  ::slog::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template<typename T>
T* CheckNotNull(slog::SourceFile file, int line, const char* names, T* ptr) {
  if (ptr == NULL) {
    Logger(file, line, LogLevel::kFatal).Stream() << names;
  }
  return ptr;
}

}  // namespace slog
