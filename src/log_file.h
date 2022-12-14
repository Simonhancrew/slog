#pragma once

#include <memory>
#include <mutex>

#include "file_utils.h"
#include "noncopyable.h"
#include "types.h"

namespace slog {

class LogFile : Noncopyable {
 public:
  LogFile(const std::string& basename, off_t roll_size, bool thread_safe = true,
          int flush_interval = 3, int check_every_n = 1024);
  ~LogFile();
  void Append(const char* file_name, int len);
  void Flush();
  bool RollFile();

 private:
  void AppendUnlocked(const char* logline, int len);

  // a NowMS() is needed, should be uint64_t
  static std::string GetLogFileName(const std::string& basename, uint64_t* now);

  const string basename_;
  const off_t roll_size_;
  const int flush_interval_;
  const int check_every_n_;

  int count_;
  std::unique_ptr<std::mutex> mutex_;
  time_t start_period_;
  time_t last_roll_;
  time_t last_flush_;
  std::unique_ptr<AppendFile> file_;
  const static int kRollPerSeconds = 60 * 60 * 24;
};

}  // namespace slog
