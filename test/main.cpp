#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#include <chrono>
#include <string>
#include <thread>

#include "async_logging.h"
#include "logging.h"
#include "time_utils_base.h"
#include "timestamp.h"

namespace slog {
AsyncLogging* log_impl = nullptr;

void AsyncOutput(const char* msg, int len) {
  log_impl->Append(msg, len);
}

void Bench(bool longLog) {
  Logger::SetOutput(AsyncOutput);
  // Logger::SetLogLevel(slog::LogLevel::kInfo);
  printf("%d\n",static_cast<int>(slog::global_level));
  int cnt           = 0;
  const int kBatch  = 1000;
  std::string empty = " ";
  std::string long_str(3000, 'X');
  long_str += " ";

  for (int t = 0; t < 30; ++t) {
    // Timestamp start = Timestamp::Now();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz " << (longLog ? long_str : empty)
               << cnt;
      ++cnt;
    }
    // slog::Timestamp end = slog::Timestamp::Now();
    // printf("%f\n", TimeDifference(end, start) * 1000000 / kBatch);
    // std::this_thread::sleep_for(std::chrono::nanoseconds(500 * 1000 * 1000));
  }
}

}  // namespace slog

std::string GetBaseName(const char* name) {
  std::string file_path{name};
  int length = file_path.length(), start = 0;
  for (int i = length - 1; i >= 0; i--) {
    if (file_path[i] == '/' || file_path[i] == '\\') {
      start = i + 1;
      break;
    }
  }
  return file_path.substr(start);
}

int main() {
  printf("pid = %d\n", _getpid());
  const off_t kRollSize = 500 * 1000 * 1000;
  char name[256]        = {'\0'};
  strncpy(name, "./test", sizeof name - 1);
  slog::AsyncLogging log(GetBaseName(name), kRollSize);
  auto start = slog::NowMs();
  log.Start();
  slog::log_impl = &log;
  slog::Bench(false);
  auto end      = slog::NowMs();
  int seconds = (end - start) / 1000, ms = (end - start) % 1000;
  printf("total use,seconds:%d\n, millisecond: %d", seconds, ms);
  return 0;
}
