
/** * @file usb_logger_c_api.cpp
 * @brief C API for UsbLogger class
 * @version 1.0
 * @date 2025-08-07
 * @ingroup UsbLogger
 * This file provides a C-style interface for the UsbLogger class, allowing
 * logging messages from C code without requiring C++ linkage.
 * @{
 */

#include "usb_logger_c_api.h"
#include "usb_logger.h"

/** * @brief C API function to log messages using UsbLogger.
 *
 * This function provides a C-style interface for logging messages
 * using the UsbLogger class, which operates in a separate thread
 * and sends messages over USB CDC.
 * @param msg Pointer to the message string to be logged.
 */
extern "C" void usb_logger_c_api(const char *msg) {
  if (msg == nullptr) {
    return; // Safety check for null message
  }
  UsbLogger::getInstance().log(msg);
}

/** @} */ // end of UsbLogger C API
