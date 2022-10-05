#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>

#include "noncopyable.h"

namespace slog {

template<typename T>
class BlockingQueue : Noncopyable {
 public:
  // default is OK
  BlockingQueue() = default;

  void Put(const T& x) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(x);
    not_empty_.notify_all();  // wait morphing saves us
    // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
  }

  void Put(T&& x) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(std::move(x));
    not_empty_.notify_all();
  }

  T Take() {
    std::unique_lock<std::mutex> lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while (queue_.empty()) {
      not_empty_.wait(lock);
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    return front;
  }

  size_t Size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::deque<T> queue_;
};

}  // namespace slog
