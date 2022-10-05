#include <stdio.h>

#include <chrono>
#include <string>
#include <thread>

#include "async_logging_test.h"

std::string GetBaseName(const char* name) {
  std::string file_path{name};
  size_t length = file_path.length(), start = 0;
  for (int i = length - 1; i >= 0; i--) {
    if (file_path[i] == '\/' || file_path[i] == '\\') {
      start = i + 1;
      break;
    }
  }
  return file_path.substr(start);
}

int main(int argc, char* argv[]) {
  printf("pid = %d\n", getpid());
  const off_t kRollSize = 500 * 1000 * 1000;
  char name[256]        = {'\0'};
  strncpy(name, argv[0], sizeof name - 1);
  slog::AsyncLogging log(GetBaseName(name), kRollSize);
  log.Start();
  slog::log_impl = &log;

  bool longLog = argc > 1;
  slog::Bench(longLog);
}
