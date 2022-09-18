#pragmae once
#include "timestamp.h"
#include "timezone.h"

namespace slog {

enum class LogLevel {
  kInfo,
  kDebug,
  kWarrn,
  kError,
  kFatal,
};

// compile time calculation of basename of source file
class SourceFile {
 public:
  template<int N>
  SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
    const char *slash = strrchr(data_, '/');  // builtin function
    if (slash) {
      data_ = slash + 1;
      size_ -= static_cast<int>(data_ - arr);
    }
  }

  explicit SourceFile(const char *filename) : data_(filename) {
    const char *slash = strrchr(filename, '/');
    if (slash) {
      data_ = slash + 1;
    }
    size_ = static_cast<int>(strlen(data_));
  }

  const char *data_;
  int size_;
};

class Impl {
 public:
  Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
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
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream &Stream() {
    return impl_.stream_;
  }

  static LogLevel LogLevel();
  static void SetLogLevel(LogLevel level);

  typedef void (*OutputFunc)(const char *msg, int len);
  typedef void (*FlushFunc)();
  static void SetOutput(OutputFunc);
  static void SetFlush(FlushFunc);
  static void SetTimeZone(const TimeZone &tz);

 private:
  Impl impl_;
};

}  // namespace slog
