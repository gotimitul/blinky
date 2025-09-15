/**
 * @file log_router.h
 * @brief Routes log messages to USB and/or filesystem sinks.
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup Logger
 * @details This file provides the interface for the LogRouter class,
 *          which is responsible for routing log messages to the appropriate
 *          logging sinks (USB and filesystem).
 * @{
 */

#ifndef LOG_ROUTER_H
#define LOG_ROUTER_H

#include <stdint.h>
#include <string_view> // For std::string_view

#ifdef __cplusplus
/**
 * @class LogRouter
 * @ingroup LogRouter
 * @brief Singleton facade that forwards logs to enabled sinks.
 *
 * @details
 * Priority of forwarding may be implementation-defined; current logic prefers
 * FS when enabled, else USB.
 */
class LogRouter {
public:
  /** @brief Get the singleton instance. */
  static LogRouter &getInstance();

  /** @brief Enable or disable USB logging. */
  void enableUsbLogging(bool enable);

  /** @brief Enable or disable filesystem logging. */
  void enableFsLogging(bool enable);

  /** @name Logging API */
  ///@{
  void log(std::string_view msg);
  void log(std::string_view msg, uint32_t val);
  void log(std::string_view msg, std::string_view str);
  void log(std::string_view msg, std::string_view str, uint32_t val);
  void log(std::string_view msg, std::string_view str, std::string_view str2,
           uint32_t val);
  ///@}

  /** @brief If FS logging is enabled, request replay of FS logs to USB. */
  void replayFsLogsToUsb();

private:
  LogRouter();
  LogRouter(const LogRouter &) = delete;
  LogRouter &operator=(const LogRouter &) = delete;
  ~LogRouter() = default;

  bool usbLoggingEnabled = false; /**< USB sink flag. */
  bool fsLoggingEnabled = false;  /**< FS sink flag. */
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif // LOG_ROUTER_H

/** @} */ // end of group Logger
