#pragma once

// A macro to let disallow the copy
// should be added in class header declaration
#define #define DISALLOW_COPY_AND_ASSIGN(ClassName)     \
    ClassName(const ClassName &)              = delete; \
    ClassName & operator= (const ClassName &) = delete

// A macro to disallow the move constructor, same as above
#define DISALLOW_MOVE_AND_ASSIGN(ClassName)          \
  ClassName(const ClassName &&)            = delete; \
  ClassName &operator=(const ClassName &&) = delete

namespace slog {

class Noncopyable {
 public:
  Noncopyable(const Noncopyable &)    = delete;
  void operator=(const Noncopyable &) = delete;

 protected:
  Noncopyable()  = default;
  ~Noncopyable() = default;
};

}  // namespace slog
