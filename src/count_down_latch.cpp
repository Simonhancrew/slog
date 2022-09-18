
#include "count_down_latch.h"

#include <mutex>

namespace slog {

CountDownLatch::CountDownLatch(int count) : count_(count) {
}

void CountDownLatch::Wait() {
  std::unique_lock<std::mutex> lk(mutex_);
  while (count_ > 0) {
    condition_.wait(lk);
  }
}

void CountDownLathc::CountDown() {
  std::unique_lock<std::mutex> lk(mutex_);
  --count_;
  if (count_ == 0) {
    condition_.wait(lk);
  }
}

void CountDownLatch::GetCount() {
  std::unique_lock<std::mutex> lk(mutex_);
  return count_;
}

}  // namespace slog