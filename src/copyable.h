#pragma once

namespace slog {

// derive class  should be value type
class copyable {
 protected:
  copyable()  = default;
  ~copyable() = default;
};

}  // namespace slog