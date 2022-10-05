#pragma once
namespace slog {

// derive class  should be value type
class Copyable {
 protected:
  Copyable()  = default;
  ~Copyable() = default;
};

}  // namespace slog
