/**
 * @file boot_clock.cpp
 * @brief Implementation of Time class for system timekeeping
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup boot_clock
 * This file implements the Time class which provides system time in a
 * human-readable format. It uses the RTOS tick count to calculate hours,
 * minutes, seconds, and milliseconds since system start.
 */

#include "boot_clock.h"
#include "cmsis_os2.h"
#include <cstdint>
#include <cstdio>

namespace {} // namespace

/** @brief Get the singleton instance of Time
 * This method returns a reference to the single instance of the Time class.
 * @return Reference to the Time instance.
 */
Time &Time::getInstance() {
  static Time instance;
  return instance;
}

/** @brief Get the current time as a formatted string.
 * The time is formatted as "HH:MM:SS.mmm" where HH is hours, MM is minutes,
 * SS is seconds, and mmm is milliseconds since system start.
 * @return Pointer to a static string containing the formatted time.
 */
char *Time::getCurrentTimeString(void) {
  std::uint32_t totalMilliseconds = osKernelGetTickCount();
  std::uint32_t hours = (totalMilliseconds / 3600000) % 24;
  std::uint32_t minutes = (totalMilliseconds / 60000) % 60;
  std::uint32_t seconds = (totalMilliseconds / 1000) % 60;
  std::uint32_t milliseconds = totalMilliseconds % 1000;

  snprintf(timeString, sizeof(timeString), "%02u:%02u:%02u.%03u", hours,
           minutes, seconds, milliseconds);
  return timeString;
}