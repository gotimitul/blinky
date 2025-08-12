
/**
 * @file usb_logger_c_api.h
 * @brief C API for USB logger functionality
 * @version 1.0
 * @date 2025-08-07
 * @ingroup UsbLogger
 *
 * This file provides a C-style interface for the UsbLogger class, allowing
 * logging messages from C code without requiring C++ linkage.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

void usb_logger_c_api(const char *msg); // C-style function to log a message

#ifdef __cplusplus
}
#endif

/** @} */ // end of UsbLogger