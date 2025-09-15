/**
 * @file boot_clock.cpp
 * @brief Implementation of BootClock class for system timekeeping
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup boot_clock
 * @details
 * This file defines the methods of the BootClock class which provides system
 * time in a human-readable format. It uses the RTOS tick count to calculate
 * hours, minutes, seconds, and milliseconds since system start.
 * The implementation includes a method to set the RTC time based on a provided
 * string in "hh:mm:ss" format.
 */

/* Boot Clock
 ---
 # 📝 Overview
  The Boot Clock module provides a way to retrieve the current system time in a
 human-readable format. It uses the RTOS tick count to calculate the elapsed
 time since system start.

 # ⚙️ Features
 - Retrieve current system time in a human-readable format.
 - Uses RTOS tick count for accurate timekeeping.
 - Set RTC time based on "hh:mm:ss" input.

 # 📋 Usage
 To use the Boot Clock module, obtain the singleton instance using
 `BootClock::getInstance()`. Call `getCurrentTimeString()` to get the current
 time as a formatted string. To set the RTC time, use the `setRTC()` method
 with a string in "hh:mm:ss" format.

 # 🔧 Implementation Details
 The BootClock class is implemented as a singleton to ensure a single instance
 throughout the application. It uses the CMSIS-RTOS2 API to get the system tick
 count and calculates the elapsed time. The time is formatted as "HH:MM:SS.mmm"
 where HH is hours, MM is minutes, SS is seconds, and mmm is milliseconds.

 The `setRTC()` method parses a string in "hh:mm:ss" format and adjusts the
 clock offset accordingly. It validates the input to ensure correct time values.
*/

#include "boot_clock.h"
#include "cmsis_os2.h"
#include <cstdint>
#include <cstdio>
#include <string_view>

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
 * @note The returned pointer is valid until the next call to this function
 *       from any thread. Copy the string immediately if you need to retain it.
 */
std::string_view BootClock::getCurrentTimeString(void) {
  std::uint32_t totalMilliseconds =
      osKernelGetTickCount() + clock_offset.load();
  std::uint32_t hours = (totalMilliseconds / 3600000) % 24;
  std::uint32_t minutes = (totalMilliseconds / 60000) % 60;
  std::uint32_t seconds = (totalMilliseconds / 1000) % 60;
  std::uint32_t milliseconds = totalMilliseconds % 1000;

  snprintf(timeString.data(), timeString.size(), "%02u:%02u:%02u.%03u", hours,
           minutes, seconds, milliseconds); // Format time string
  return timeString.data(); // Return pointer to formatted time string
}

/** @brief Set the RTC time based on a provided string.
 * The input string should be in the format "hh:mm:ss".
 * @param buf Pointer to a string containing the time in "hh:mm:ss" format.
 * @return Status code indicating success or type of error.
 */
SetRTCStatus BootClock::setRTC(std::string_view buf) {

  std::uint32_t hours, minutes, seconds;
  if (std::sscanf(buf.data(), "%2u:%2u:%2u", &hours, &minutes, &seconds) != 3) {
    return SetRTCStatus::INVALID_RX_FORMAT; // Parsing error
  }

  if (hours >= 24 || minutes >= 60 || seconds >= 60) {
    return SetRTCStatus::INVALID_VALUE; // Invalid time values
  }

  clock_offset.store((hours * 3600000) + (minutes * 60000) + (seconds * 1000) -
                     osKernelGetTickCount()); // Adjust clock offset

  return SetRTCStatus::SUCCESS; // Success
}