#pragma once

namespace slog {

// TimeZone for 1970~2030
// use std::tm
class TimeZone {
 public:
  explicit TimeZone(const char* zone_file);
 private:
};

}  // namespace slog
