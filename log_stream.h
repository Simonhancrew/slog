#pragma once

#include "slog/noncopyable.h"
#include "slog/types.h"
#include <assert.h>
#include <string.h> // for memcpy

namespace {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

}


namespace slog {

template<int SIZE> 
class FixedBuffer: noncopyable {
 public:
   FixedBuffer(): cur_(data_) {
    SetCookie()
   }
  private:
    const char* end() const { return data_ + sizeof data_; }
    static void CookieStart();
    static void CookieEnd();


    void (*cookie_)();
    char data_[SIZE];
    char* cur_;  
};





}
