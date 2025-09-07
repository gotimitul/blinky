/**
 * @file log_router.cpp
 * @brief LogRouter implementation for routing log messages
 * to USB and filesystem.
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup LogRouter
 */

/* Log Router
 ---
 The Log Router is responsible for routing log messages to the appropriate
 logging mechanism, either USB or file system. It manages the logging
 state and provides a unified interface for logging.

  # 📝 Overview
  The Log Router provides a way to route log messages to either USB CDC or the
 file system, depending on the logging state. It supports enabling/disabling
 logging for each mechanism and ensures that log messages are sent to the
 correct destination. # ⚙️ Features
  - Route log messages to USB CDC or file system.
  - Enable/disable logging for each mechanism.
  - Unified logging interface.
  - Thread-safe logging using RTOS primitives.
  - Integration with existing logging classes (UsbLogger and FsLog).
  - Support for formatted log messages with variable arguments.

  # 📋 Usage
  To use the Log Router, obtain the singleton instance using
 `LogRouter::getInstance()`. Use the `log` method to log messages, which will be
 routed based on the enabled logging mechanisms. Enable or disable logging for
 USB or file system using `enableUsbLogging` and `enableFsLogging`.

  # 🔧 Implementation Details
  The Log Router is implemented as a singleton class `LogRouter`. It maintains
 flags to track the enabled state of USB and file system logging. The `log`
 method checks these flags and routes the log message to the appropriate logging
 class. The class provides multiple overloads of the `log` method to support
 different types of log messages, including formatted strings with variable
 arguments. The Log Router integrates with the `UsbLogger` and `FsLog` classes,
 which handle the actual logging to USB and file system, respectively.
 */

#include "log_router.h"
#include "fs_log.h"
#include "logger.h"
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
  if (msg == nullptr) {
#ifdef DEBUG
    printf("LogRouter::log: msg is null: %s, %d\r\n", __FILE__, __LINE__);
#endif
    return; // Stop logging if message is null
  }
  Logger *logger;
  if (fsLoggingEnabled) {
    logger = static_cast<Logger *>(&FsLog::getInstance());
  } else if (usbLoggingEnabled) {
    logger = static_cast<Logger *>(&UsbLogger::getInstance());
  } else {
    return; // No logging enabled
  }
  logger->log(msg);
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
void LogRouter::replayFsLogsToUsb() { FsLog::getInstance().replayLogsToUsb(); }