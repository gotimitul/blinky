/**
 * @file boot_clock.h
 * @brief Declaration of Time class for system timekeeping
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup boot_clock
 * This file declares the Time class which provides system time in a
 * human-readable format. It uses the RTOS tick count to calculate hours,
 * minutes, seconds, and milliseconds since system start.
 */

#ifndef BOOT_CLOCK_H
#define BOOT_CLOCK_H

#include <cstring>

#ifdef __cplusplus

/**
 * @class Time
 * @brief Singleton class for system timekeeping
 * This class provides methods to get the current system time formatted
 * as a string. It uses the RTOS tick count to compute hours, minutes,
 * seconds, and milliseconds since system start.
 */
class Time {
public:
  static Time &getInstance(); // Get the singleton instance

  char *getCurrentTimeString(void); // Get current time as formatted string

private:
  Time() {};                   // Private constructor for singleton pattern
  Time(const Time &) = delete; // Delete copy constructor
  Time &operator=(const Time &) = delete; // Delete copy assignment operator

  char timeString[16]; // Buffer to hold formatted time string
}; // End of Time class

extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // BOOT_CLOCK_H