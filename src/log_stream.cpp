

#include "log_stream.h"

namespace {

const char digits[]    = "9876543210123456789";
const char digitsHex[] = "0123456789ABCDEF";
const char *zero       = digits + 9;

}  // namespace

namespace slog {

template<typename T>
size_t Convert(char buf[], T value) {
  T i     = value;
  char *p = buf;

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
  char *p     = buf;

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

}  // namespace slog
