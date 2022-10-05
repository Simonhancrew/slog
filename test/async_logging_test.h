#include <stdio.h>

#include <chrono>
#include <string>
#include <thread>

#include "async_logging.h"
#include "logging.h"
#include "timestamp.h"

namespace slog {
AsyncLogging* log_impl = nullptr;

void asyncOutput(const char* msg, int len) {
  log_impl->Append(msg, len);
}

void Bench(bool longLog) {
  Logger::SetOutput(asyncOutput);

  int cnt           = 0;
  const int kBatch  = 1000;
  std::string empty = " ";
  std::string long_str(3000, 'X');
  long_str += " ";

  for (int t = 0; t < 30; ++t) {
    Timestamp start = Timestamp::Now();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz " << (longLog ? long_str : empty)
               << cnt;
      ++cnt;
    }
    slog::Timestamp end = slog::Timestamp::Now();
    printf("%f\n", TimeDifference(end, start) * 1000000 / kBatch);
    std::this_thread::sleep_for(std::chrono::nanoseconds(500 * 1000 * 1000));
  }
}
}  // namespace slog
