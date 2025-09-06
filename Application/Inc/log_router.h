
/**
 * @file log_router.h
 * @brief Header file for LogRouter class to route log messages to USB and
 * filesystem.
 * @author Mitul Goti
 * @date 2025-08-07
 * @version 1.0
 * @note This class is designed to be thread-safe.
 * @see LogRouter::getInstance()
 */

#ifndef LOG_ROUTER_H
#define LOG_ROUTER_H

#include "stdint.h"

#ifdef __cplusplus

/**
 * LogRouter is a singleton class responsible for routing log messages
 * to different output channels (e.g., USB, filesystem).
 * It provides methods to enable or disable logging to specific channels
 * and to log messages with various formats.
 * It also includes a method to replay filesystem logs to USB.
 */
class LogRouter {
public:
  static LogRouter &getInstance(); // Get the singleton instance

  void enableUsbLogging(bool enable); // Enable or disable USB logging
  void enableFsLogging(bool enable);  // Enable or disable filesystem logging

  void log(const char *msg); // Log a simple message

  void log(const char *msg,
           uint32_t val); // Log a message with a single integer value

  void log(const char *msg,
           const char *str); // Log a message with a single string

  void log(const char *msg, const char *str,
           uint32_t val); // Log a message with a string and an integer

  void log(const char *msg, const char *str, const char *str2,
           uint32_t val); // Log a message with two strings and an integer

  void replayFsLogsToUsb(); // Replay filesystem logs to USB

private:
  LogRouter(); // Private constructor for singleton pattern
  LogRouter(const LogRouter &) = delete;
  LogRouter &operator=(const LogRouter &) = delete;
  ~LogRouter() = default;

  bool usbLoggingEnabled = false; // USB logging is disabled by default
  bool fsLoggingEnabled = false;  // Filesystem logging is disabled by default
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif // LOG_ROUTER_H