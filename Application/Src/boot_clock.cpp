/**
 * @file boot_clock.cpp
 * @brief Implementation of BootClock class for system timekeeping
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup boot_clock
 * @details
 * This file implements the BootClock class which provides system time in a
 * human-readable format. It uses the RTOS tick count to calculate hours,
 * minutes, seconds, and milliseconds since system start.
 */

/* Boot Clock
 ---
 # 📝 Overview
 The Boot Clock module provides a way to keep track of system time since boot.
 It uses the RTOS tick count to calculate the elapsed time in hours, minutes,
 seconds, and milliseconds.

 # ⚙️ Features
 - Retrieve current system time in a human-readable format.
 - Uses RTOS tick count for accurate timekeeping.

 # 📋 Usage
 To use the Boot Clock module, obtain the singleton instance using
 `BootClock::getInstance()`. Call `getCurrentTimeString()` to get the current
 time as a formatted string.

 # 🔧 Implementation Details
 The BootClock class is implemented as a singleton to ensure a single instance
 throughout the application. It uses the CMSIS-RTOS2 API to get the system tick
 count and calculates the elapsed time. The time is formatted as "HH:MM:SS.mmm"
 where HH is hours, MM is minutes, SS is seconds, and mmm is milliseconds.
*/

#include "boot_clock.h"
#include "cmsis_os2.h"
#include <cstdint>
#include <cstdio>

/** @brief Get the singleton instance of BootClock
 * This method returns a reference to the single instance of the BootClock
 * class.
 * @return Reference to the BootClock instance.
 */
BootClock &BootClock::getInstance() {
  static BootClock instance;
  return instance;
}

/** @brief Get the current time as a formatted string.
 * The time is formatted as "HH:MM:SS.mmm" where HH is hours, MM is minutes,
 * SS is seconds, and mmm is milliseconds since system start.
 * @return Pointer to a static string containing the formatted time.
 */
char *BootClock::getCurrentTimeString(void) {
  std::uint32_t totalMilliseconds = osKernelGetTickCount();
  std::uint32_t hours = (totalMilliseconds / 3600000) % 24;
  std::uint32_t minutes = (totalMilliseconds / 60000) % 60;
  std::uint32_t seconds = (totalMilliseconds / 1000) % 60;
  std::uint32_t milliseconds = totalMilliseconds % 1000;

  snprintf(timeString, sizeof(timeString), "%02u:%02u:%02u.%03u", hours,
           minutes, seconds, milliseconds);
  return timeString;
}