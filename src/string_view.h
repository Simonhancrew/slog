#pragma once

#include <string>

namespace slog {

// make sure the string or char* live longer than stringview
class StringView  // copyable
{
 public:
  StringView(const char *str) : str_(str) {
  }

  StringView(const std::string &str) : str_(str.c_str()) {
  }

  const char *c_str() const {
    return str_;
  }

 private:
  const char *str_;
};

}  // namespace slog
