#pragma once

#include <cstdint>
#include <ctime>
#include <string>

namespace slog {

uint64_t TickMs();

uint64_t NowMs();

int32_t LocalTime(const std::time_t &time, std::tm *tm);

std::tm LocalTime();

std::string LocalTimeString();

}  // namespace slog