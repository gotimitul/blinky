/**
 * @file boot_clock.h
 * @brief Declaration of BootClock class for system timekeeping
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @defgroup boot_clock boot_clock
 * @{
 * @details
 * This file declares the BootClock class which provides system time in a
 * human-readable format. It uses the RTOS tick count to calculate hours,
 * minutes, seconds, and milliseconds since system start.
 */

#ifndef BOOT_CLOCK_H
#define BOOT_CLOCK_H

#include <array>
#include <atomic>
#include <cstring>
#include <string_view>

#ifdef __cplusplus

/**
 * @class BootClock
 * @brief Singleton class for system timekeeping
 * This class provides methods to get the current system time formatted
 * as a string. It uses the RTOS tick count to compute hours, minutes,
 * seconds, and milliseconds since system start.
 * It also provides a method to set the RTC time based on a provided string.
 */
class BootClock {
public:
  enum class SetRTCStatus : std::int8_t {
    SUCCESS = 0,
    INVALID_RX_FORMAT = -1,
    INVALID_VALUE = -2,
  };

  SetRTCStatus setRTC(std::string_view buf);

  static BootClock &getInstance(); ///< Get singleton instance

  std::string_view
  getCurrentTimeString(void); ///< Get current time as formatted string

private:
  BootClock() = default; ///< Private constructor for singleton pattern
  ~BootClock() = default;

  constexpr static size_t TIME_STRING_SIZE = 16;
  std::array<char, TIME_STRING_SIZE>
      timeString; ///< Buffer to hold formatted time string

  std::atomic_uint32_t clock_offset = 0; ///< Offset to adjust clock time
}; // End of BootClock class

extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif    // BOOT_CLOCK_H
/** @} */ // end of boot_clock