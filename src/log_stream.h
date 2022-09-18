#pragma once

#include <assert.h>
#include <string.h>  // for memcpy

#include "slog/noncopyable.h"
#include "slog/types.h"

namespace {

const size_t kSmallBuffer = 4000;
const size_t kLargeBuffer = 4000 * 1000;
const size_t kBufSize     = 32;

}  // namespace

namespace slog {

template<int SIZE>
class FixedBuffer : noncopyable {
 public:
  FixedBuffer() : cur_(data_) {
    SetCookie(CookieStart);
  }
  ~FixedBuffer() {
    SetCookie(CookieEnd);
  }

  void Append(const char *data, size_t len) {
    if (implicit_cast<size_t>(avail()) > len) {
      memcpy(cur_, data, len);
      cur_ += len;
    }
  }

  const char *Data() const {
    return data_;
  }

  size_t Length() {
    return static_cast<size_t>(cur_ - data_);
  }

  char *Current() {
    return cur_;
  }
  size_t Avail() const {
    return static_cast<size_t>(End() - cur_);
  }

  void Add(size_t len) {
    cur_ += len;
  }

  void Reset() {
    cur_ = data_;
  }

  void Bzero() {
    MemZero(data_, sizeof data_);
  }

  // for GDB
  const char *DebugString();
  void SetCookie(void (*cookie)()) {
    cookie_ = cookie;
  }

  // for UT
  std::string ToString() const {
    return std::string(data_, Length());
  }

 private:
  const char *End() const {
    return data_ + sizeof data_;
  }
  static void CookieStart();
  static void CookieEnd();

  void (*cookie_)();
  char data_[SIZE];
  char *cur_;
};

class LogStream : noncopyable {
  using self = LogStream;

 public:
  using Buffer = FixedBuffer<kSmallBuffer>;

  self &operator<<(bool v) {
  }

 private:
  void staticCheck();

  template<typename T>
  void FormatInt(T);

  Buffer buffer_;
  static const int kMaxNumericSize = 48;
};

class Fmt {
 public:
  template<typename T>
  Fmt(const char *fmt, T val);

  const char *Data() const {
    return buf_;
  }
  size_t Length() const {
    return length_;
  }

 private:
  char buf_[kBufSize];
  size_t length_;
};

}  // namespace slog
