

#include "waitable_events.h"

#include <condition_variable>
#include <mutex>

namespace slog {

void AutoResetEvent::Set() {
  std::lock_guard<std::mutex> _(lock_);
  signal_ = true;

  cv_.notify_one();
}

int AutoResetEvent::Wait(int wait_ms) {
  int res = -1;

  std::unique_lock<std::mutex> lk(lock_);

  if (signal_) {
    signal_ = false;
    return 0;
  }

  if (wait_ms == 0) {
    return -1;
  }

  if (wait_ms < 0) {
    cv_.wait(lk, [this] {
      return signal_;
    });
    res = 0;
  } else {
    auto wait_succeeded =
        cv_.wait_for(lk, std::chrono::milliseconds(wait_ms), [this] {
          return signal_;
        });
    res = (wait_succeeded ? 0 : -1);
  }

  // here is the auto reset
  signal_ = false;
  return res;
}

}  // namespace slog
