#pragma once
#include "noncopyable.h"
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace slog{

class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void Wait();

  void CountDown();

  int GetCount() const;

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::atomic<int> count_;
};


}