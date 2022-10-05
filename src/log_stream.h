#pragma once

#include <assert.h>
#include <string.h>  // for memcpy

#include "noncopyable.h"
#include "types.h"

namespace {

const size_t kSmallBuffer = 4000;
const size_t kLargeBuffer = 4000 * 1000;

}  // namespace

namespace slog {
// simplify packer
template<int SIZE>
class FixedBuffer : Noncopyable {
 public:
  FixedBuffer() : cur_(data_) {
    SetCookie(CookieStart);
  }
  ~FixedBuffer() {
    SetCookie(CookieEnd);
  }

  size_t Avail() const {
    return static_cast<size_t>(End() - cur_);
  }

  void Append(const char* data, size_t len) {
    if (Implicit_Cast<size_t>(Avail()) > len) {
      memcpy(cur_, data, len);
      cur_ += len;
    }
  }

  const char* Data() const {
    return data_;
  }

  size_t Length() const {
    return static_cast<size_t>(cur_ - data_);
  }

  char* Current() {
    return cur_;
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
  // const char* DebugString();
  void SetCookie(void (*cookie)()) {
    cookie_ = cookie;
  }

  // for UT
  std::string ToString() const {
    return std::string(data_, Length());
  }

 private:
  const char* End() const {
    return data_ + sizeof data_;
  }
  // for GDB
  static void CookieStart();
  static void CookieEnd();

  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};

class LogStream : Noncopyable {
  using self = LogStream;

 public:
  using Buffer = FixedBuffer<kSmallBuffer>;

  self& operator<<(bool v) {
    buffer_.Append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }

  self& operator<<(double);

  self& operator<<(char v) {
    buffer_.Append(&v, 1);
    return *this;
  }

  self& operator<<(const char* str) {
    if (str) {
      buffer_.Append(str, strlen(str));
    } else {
      buffer_.Append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const string& v) {
    buffer_.Append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const Buffer& v) {
    *this << v.ToString();
    return *this;
  }

  void Append(const char* data, int len) {
    buffer_.Append(data, len);
  }

  const Buffer& buffer() const {
    return buffer_;
  }

  void resetBuffer() {
    buffer_.Reset();
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
  Fmt(const char* fmt, T val);

  const char* Data() const {
    return buf_;
  }
  size_t Length() const {
    return length_;
  }

 private:
  char buf_[32];
  size_t length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
  s.Append(fmt.Data(), fmt.Length());
  return s;
}

}  // namespace slog
