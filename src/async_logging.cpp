#include "async_logging.h"

#include <Windows.h>

#include "log_file.h"
#include "timestamp.h"

namespace slog {
AsyncLogging::AsyncLogging(const string& basename, off_t roll_size,
                           int flush_interval)
    : flush_interval_(flush_interval),
      running_(false),
      basename_(basename),
      roll_size_(roll_size),
      latch_(1),
      mutex_(),
      current_buffer_(new Buffer),
      next_buffer_(new Buffer),
      buffers_() {
  current_buffer_->Bzero();
  next_buffer_->Bzero();
  buffers_.reserve(16);
}

void AsyncLogging::Append(const char* line, int len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (current_buffer_->Avail() > len) {
    current_buffer_->Append(line, len);
  } else {
    buffers_.push_back(std::move(current_buffer_));

    if (next_buffer_) {
      current_buffer_ = std::move(next_buffer_);
    } else {
      current_buffer_.reset(new Buffer);  // Rarely happens
    }
    current_buffer_->Append(line, len);
    cond_.notify_one();
  }
}

void AsyncLogging::ThreadFunc() {
  assert(running_ == true);
  latch_.CountDown();

  LogFile output(basename_, roll_size_, false);

  BufferPtr new_buffer1(new Buffer);
  BufferPtr new_buffer2(new Buffer);
  new_buffer1->Bzero();
  new_buffer2->Bzero();

  BufferVector buffers_to_write;
  buffers_to_write.reserve(16);

  while (running_) {
    assert(new_buffer1 && new_buffer1->Length() == 0);
    assert(new_buffer2 && new_buffer2->Length() == 0);
    assert(buffers_to_write.empty());

    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty())  // unusual usage!
      {
        cond_.wait_for(lock, std::chrono::seconds(flush_interval_));
      }
      buffers_.push_back(std::move(current_buffer_));
      current_buffer_ = std::move(new_buffer1);
      buffers_to_write.swap(buffers_);
      if (!next_buffer_) {
        next_buffer_ = std::move(new_buffer2);
      }
    }

    assert(!buffers_to_write.empty());

    if (buffers_to_write.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf,
               "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::Now().ToFormattedString().c_str(),
               buffers_to_write.size() - 2);
      fputs(buf, stderr);
      output.Append(buf, static_cast<int>(strlen(buf)));
      buffers_to_write.erase(buffers_to_write.begin() + 2,
                             buffers_to_write.end());
    }

    for (const auto& buffer : buffers_to_write) {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.Append(buffer->Data(), buffer->Length());
    }

    if (buffers_to_write.size() > 2) {
      // drop non-bzero-ed buffers, avoid trashing
      buffers_to_write.resize(2);
    }

    if (!new_buffer1) {
      assert(!buffers_to_write.empty());
      new_buffer1 = std::move(buffers_to_write.back());
      buffers_to_write.pop_back();
      new_buffer1->Reset();
    }

    if (!new_buffer2) {
      assert(!buffers_to_write.empty());
      new_buffer2 = std::move(buffers_to_write.back());
      buffers_to_write.pop_back();
      new_buffer2->Reset();
    }

    buffers_to_write.clear();
    output.Flush();
  }
  output.Flush();
}
}  // namespace slog