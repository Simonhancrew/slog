#pragma once

#include "noncopyable.h"

namespace slog {

class AutoResetEvent : Noncopyable {
 public:
  explicit AutoResetEvent(bool init = false) : signal_(init) {
  }
  ~AutoResetEvent() = default;

  void Set();
  int Wait(int wait_ms = -1);

 private:
  std::condition_variable cv_;
  std::mutex lock_;
  bool signal_;
};

}  // namespace slog