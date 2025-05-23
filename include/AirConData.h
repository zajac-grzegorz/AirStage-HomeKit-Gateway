#pragma once

#include <atomic>

enum class OperationMode
{
    OP_AUTO = 0,
    OP_COOL = 1,
    OP_DRY = 2,
    OP_FAN = 3,
    OP_HEAT = 4
};

enum class AirConMode
{
   AC_OFF = 0,
   AC_HEAT = 1,
   AC_COOL = 2,
   AC_AUTO = 3,
};

enum class FanSpeed
{
   FS_AUTO = 0,
   FS_QUIET = 2,
   FS_LOW = 5,
   FS_MEDIUM = 8,
   FS_HIGH = 11
};

inline std::atomic<int> acInsideTemp(0);
inline std::atomic<int> acOutsideTemp(0);
inline std::atomic<int> acSetTemp(0);
inline std::atomic<int> acFanSpeed((int) FanSpeed::FS_AUTO);
inline std::atomic<int> acAirConMode((int) AirConMode::AC_OFF);
inline std::atomic_flag acDryMode(false);
inline std::atomic_flag acFanOnlyMode(false);

