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

#include <cstdint>
#include <cstring>

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
  static BootClock &getInstance(); ///< Get singleton instance

  char *getCurrentTimeString(void); ///< Get current time as formatted string

  std::int32_t setRTC(char *buf);

private:
  BootClock() {}; ///< Private constructor for singleton pattern
  BootClock(const BootClock &) = delete; ///< Delete copy constructor
  BootClock &
  operator=(const BootClock &) = delete; ///< Delete copy assignment operator

  char timeString[16]; ///< Buffer to hold formatted time string

  std::uint32_t clock_offset = 0; ///< Offset to adjust clock time
}; // End of BootClock class

extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif    // BOOT_CLOCK_H
/** @} */ // end of boot_clock