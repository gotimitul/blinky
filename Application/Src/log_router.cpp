/**
 * @file log_router.cpp
 * @brief LogRouter implementation for routing log messages
 * @version 1.0
 * @date 2025-08-07
 * @ingroup LogRouter
 */

#include "log_router.h"
#include "fs_log.h"
#include "usb_logger.h"
#include <cstdio>

/** @brief Get the singleton instance of LogRouter
 * This method returns a reference to the single instance of the LogRouter
 * class, creating it if it does not already exist.
 */
LogRouter &LogRouter::getInstance() {
  static LogRouter instance;
  return instance;
}

/** @brief Constructor for LogRouter
 * Initializes the logging flags to false.
 */
LogRouter::LogRouter() : usbLoggingEnabled(false), fsLoggingEnabled(false) {}

/** @brief Enable or disable USB logging.
 * @param enable True to enable USB logging, false to disable it.
 */
void LogRouter::enableUsbLogging(bool enable) { usbLoggingEnabled = enable; }

/** @brief Enable or disable file system logging.
 * @param enable True to enable file system logging, false to disable it.
 */
void LogRouter::enableFsLogging(bool enable) { fsLoggingEnabled = enable; }

/** @brief Log a simple message.
 * Routes the message to the appropriate logging mechanism based on
 * the enabled flags.
 * @param msg The message string to log.
 */
void LogRouter::log(const char *msg) {
  using LogFunc = void (*)(const char *); // Function pointer type for logging
  LogFunc logFunc = nullptr;              // Initialize to null
  // Determine which logging mechanism to use
  // Priority: File system logging if enabled, otherwise USB logging
  if (fsLoggingEnabled) {
    logFunc = [](const char *m) { FsLog::getInstance().log(m); };
  } else if (usbLoggingEnabled) {
    logFunc = [](const char *m) { UsbLogger::getInstance().log(m); };
  }
  // Call the selected logging function if available
  if (logFunc) {
    logFunc(msg);
  }
}

/** @brief Log a message with an integer value.
 * @param msg Format string for the log message.
 * @param val Integer value to include in the log message.
 */
void LogRouter::log(const char *msg, uint32_t val) {
  char logBuffer[64];
  snprintf(logBuffer, sizeof(logBuffer), msg, val);
  log(logBuffer);
}

/** @brief Log a message with a string value.
 * @param msg Format string for the log message.
 * @param str String value to include in the log message.
 */
void LogRouter::log(const char *msg, const char *str) {
  char logBuffer[128];
  snprintf(logBuffer, sizeof(logBuffer), msg, str);
  log(logBuffer);
}

/** @brief Log a message with a string and an integer value.
 * @param msg Format string for the log message.
 * @param str String value to include in the log message.
 * @param val Integer value to include in the log message.
 */
void LogRouter::log(const char *msg, const char *str, uint32_t val) {
  char logBuffer[128];
  snprintf(logBuffer, sizeof(logBuffer), msg, str, val);
  log(logBuffer);
}

/** @brief Log a message with two strings and an integer value.
 * @param msg Format string for the log message.
 * @param str First string value to include in the log message.
 * @param str2 Second string value to include in the log message.
 * @param val Integer value to include in the log message.
 */
void LogRouter::log(const char *msg, const char *str, const char *str2,
                    uint32_t val) {
  char logBuffer[256];
  snprintf(logBuffer, sizeof(logBuffer), msg, str, str2, val);
  log(logBuffer);
}

/** @brief Replay filesystem logs to USB if filesystem logging is enabled.
 */
void LogRouter::replayFsLogsToUsb() {
  if (fsLoggingEnabled) { /* Replay FS logs to USB */
    FsLog::getInstance().replayLogsToUsb();
  }
}