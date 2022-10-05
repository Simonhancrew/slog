

#include "log_stream.h"

namespace {

const char digits[]    = "9876543210123456789";
const char digitsHex[] = "0123456789ABCDEF";
const char* zero       = digits + 9;

}  // namespace

namespace slog {

template<typename T>
size_t Convert(char buf[], T value) {
  T i     = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

size_t ConvertHex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char* p     = buf;

  do {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template<int SIZE>
void FixedBuffer<SIZE>::CookieStart() {
}

template<int SIZE>
void FixedBuffer<SIZE>::CookieEnd() {
}

template<typename T>
void LogStream::FormatInt(T v) {
  if (buffer_.Avail() >= kMaxNumericSize) {
    size_t len = Convert(buffer_.Current(), v);
    buffer_.Add(len);
  }
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  FormatInt(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  FormatInt(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  FormatInt(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  FormatInt(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  FormatInt(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  FormatInt(v);
  return *this;
}

LogStream& LogStream::operator<<(const void* p) {
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (buffer_.Avail() >= kMaxNumericSize) {
    char* buf  = buffer_.Current();
    buf[0]     = '0';
    buf[1]     = 'x';
    size_t len = ConvertHex(buf + 2, v);
    buffer_.Add(len + 2);
  }
  return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buffer_.Avail() >= kMaxNumericSize) {
    int len = snprintf(buffer_.Current(), kMaxNumericSize, "%.12g", v);
    buffer_.Add(len);
  }
  return *this;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val) {
  static_assert(std::is_arithmetic<T>::value == true,
                "Must be arithmetic type");

  length_ = snprintf(buf_, sizeof buf_, fmt, val);
  assert(static_cast<size_t>(length_) < sizeof buf_);
}

// Explicit instantiations
template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);

}  // namespace slog
