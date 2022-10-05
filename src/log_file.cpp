
#include "log_file.h"

#include <assert.h>

#include <string>

#include "time_utils_base.h"

namespace slog {
LogFile::LogFile(const std::string& basename, off_t roll_size, bool thread_safe,
                 int flush_interval, int check_time)
    : basename_(basename),
      roll_size_(roll_size),
      flush_interval_(flush_interval),
      check_every_n_(check_time),
      count_(0),
      mutex_(thread_safe ? new std::mutex : nullptr),
      start_period_(0),
      last_roll_(0),
      last_flush_(0) {
  RollFile();
}

LogFile::~LogFile() = default;

void LogFile::Append(const char* log_line, int len) {
  if (mutex_) {
    std::unique_lock<std::mutex> lock(*(mutex_.get()));
    AppendUnlocked(log_line, len);
  } else {
    AppendUnlocked(log_line, len);
  }
}

void LogFile::Flush() {
  if (mutex_) {
    std::unique_lock<std::mutex> lock(*(mutex_.get()));
    file_->Flush();
  } else {
    file_->Flush();
  }
}

void LogFile::AppendUnlocked(const char* log_line, int len) {
  file_->Append(log_line, len);

  if (file_->WrittenBytes() > roll_size_) {
    RollFile();
  } else {
    ++count_;
    auto now         = NowMs();
    auto this_period = now / kRollPerSeconds * kRollPerSeconds;
    if (this_period != start_period_) {
      RollFile();
    } else if (now - last_flush_ > flush_interval_) {
      last_flush_ = now;
      file_->Flush();
    }
  }
}

bool LogFile::RollFile() {
  auto now              = NowMs();
  std::string file_name = GetLogFileName(basename_, &now);
  auto start            = now / kRollPerSeconds * kRollPerSeconds;
  if (now > last_roll_) {
    last_roll_    = now;
    last_flush_   = now;
    start_period_ = start;
    file_.reset(new AppendFile(file_name));
    return true;
  }
  return false;
}

std::string GetLogFileName(const std::string& basename, uint64_t* now) {
  string file_name;
  file_name.reserve(basename.size() + 64);
  file_name = basename;

  char timebuf[32];

  std::chrono::system_clock::time_point now_point =
      std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now_point);
  std::tm* now_tm        = std::localtime(&now_time_t);
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", now_tm);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now_point.time_since_epoch()) %
            1000;

  file_name += timebuf;
  file_name += ":" + std::to_string(ms.count());
  file_name += ".log";

  return file_name;
}
}  // namespace slog
