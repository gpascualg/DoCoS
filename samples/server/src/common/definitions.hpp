#pragma once

#include <inttypes.h>
#include <chrono>


using std_clock_t = std::chrono::steady_clock;
using sys_clock_t = std::chrono::system_clock;
using base_time = std::chrono::milliseconds;
constexpr inline base_time HeartBeat = base_time(std::chrono::milliseconds(50));

inline base_time elapsed(std_clock_t::time_point from, std_clock_t::time_point to)
{
    return std::chrono::duration_cast<base_time>(to - from);
}

