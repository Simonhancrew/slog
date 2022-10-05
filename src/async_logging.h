#pragma once

#include <Windows.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "count_down_latch.h"
#include "log_stream.h"
#include "noncopyable.h"

namespace slog {

class AsyncLogging : Noncopyable {
 public:
  AsyncLogging(const std::string& basename, off_t roll_size,
               int flush_interval = 3);

  ~AsyncLogging() {
    if (running_) {
      Stop();
    }
  }

  void Append(const char* logline, int len);

  void Start() {
    running_ = true;
    thread_  = std::thread([this] {
      SetThreadDescription(GetCurrentThread(), L"Log");
      ThreadFunc();
    });
    latch_.Wait();
  }

  void Stop() {
    running_ = false;
    cond_.notify_all();
    thread_.join();
  }

 private:
  void ThreadFunc();
  using Buffer       = FixedBuffer<kLargeBuffer>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr    = BufferVector::value_type;

  const int flush_interval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t roll_size_;
  std::thread thread_;
  CountDownLatch latch_;
  std::mutex mutex_;
  std::condition_variable cond_;
  BufferPtr current_buffer_;
  BufferPtr next_buffer_;
  BufferVector buffers_;
};

}  // namespace slog
