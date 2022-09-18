#pragma once

#include <exception>
#include <string>

namespace slog {

class Exception : public std::exception {
 public:
  Exception(std::string what);
  ~Exception() noexcept override = default;

  // default copy-ctor and operator= are okay.

  const char *what() const noexcept override {
    return message_.c_str();
  }

  const char *StackTrace() const noexcept {
    return stack_.c_str();
  }

 private:
  std::string message_;
  std::string stack_;
};

}  // namespace slog
