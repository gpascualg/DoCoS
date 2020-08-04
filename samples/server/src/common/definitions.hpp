#pragma once

#include <inttypes.h>
#include <chrono>


using std_clock_t = std::chrono::steady_clock;
using sys_clock_t = std::chrono::system_clock;
using base_time = std::chrono::milliseconds;

constexpr inline uint8_t TicksPerSecond = 40;
constexpr inline base_time HeartBeat = base_time(base_time(std::chrono::seconds(1)).count() / TicksPerSecond);

inline base_time elapsed(std_clock_t::time_point from, std_clock_t::time_point to)
{
    return std::chrono::duration_cast<base_time>(to - from);
}











// CONSTANTS USED FOR FIBER MANAGEMENT
enum class FiberID
{
    ServerWorker =      0,  // This one CANNOT be changed, it is DoCos internal
    NetworkWorker =     1,
    DatabaseWorker =    2
};







// COMMON STRUCTURES
struct udp_buffer
{
    uint8_t buffer[500]; // TODO(gpascualg): MaxSize
};
