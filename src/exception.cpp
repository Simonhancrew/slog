

#include "exception.h"

namespace slog {

// TBD: stack_ ?
Exception::Exception(std::string msg) : message_(std::move(msg)), stack_() {
}

}  // namespace slog