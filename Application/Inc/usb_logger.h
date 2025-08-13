/**
 * @file usb_logger.h
 * @brief UsbLogger class for USB CDC-based logging (CMSIS-RTOS2)
 * @version 1.0
 * @date 2025-08-07
 * @author
 *
 * @defgroup UsbLogger USB CDC Logger
 * @brief Thread-safe USB logger class for transmitting messages over USB CDC.
 * @{
 */

#ifndef USB_LOGGER_H
#define USB_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os2.h"

#ifdef __cplusplus
}
#endif

/**
 * @class UsbLogger
 * @ingroup UsbLogger
 * @brief Singleton class for USB CDC logging using CMSIS-RTOS2.
 *
 * This class provides a global thread-safe logger that uses a background thread
 * to send messages over USB CDC interface.
 */
class UsbLogger {
public:
  /// @name Public API
  /// @{

  static UsbLogger &
  getInstance(); // Returns the singleton instance of UsbLogger.

  void init(); // Initializes the logger's message queue.

  void log(const char *msg); // Logs a message to the USB CDC interface.

  void log(const char *msg, uint32_t val); // Logs a message with an integer
                                           // value to the USB CDC interface.

  void log(const char *msg,
           const char *str); // Logs a message with a string value to the USB
                             // CDC interface.

  void log(const char *msg, const char *str,
           uint32_t val); // Logs a message with a string and an integer value
                          // to the USB CDC interface.
                          // @}

private:
  /// @name Internal Mechanics
  /// @{

  UsbLogger(); ///< Private constructor for singleton pattern.
  UsbLogger(const UsbLogger &) =
      delete; ///< Delete copy constructor to prevent copying.
  UsbLogger &operator=(const UsbLogger &) =
      delete; ///< Delete assignment operator to prevent assignment.

  static void
  loggerThreadWrapper(void *argument); // Static wrapper to call loggerThread
                                       // from C-style function pointer.

  void loggerThread(); // Thread function that waits for messages and sends
                       // them via USB CDC.

  /// @}
};

#endif // USB_LOGGER_H

/** @} */ // end of UsbLogger group
