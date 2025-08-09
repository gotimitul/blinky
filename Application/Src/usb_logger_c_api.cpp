
/// @file usb_logger_c_api.cpp
/// @brief C API implementation for UsbLogger class
/// @version 1.0
/// @date 2025-08-07
/// @ingroup UsbLogger
/// @addtogroup UsbLogger
/// @{
#include    "usb_logger_c_api.h"
#include    "usb_logger.h"


/** * @brief C API function to log messages using UsbLogger.
    * 
    * This function provides a C-style interface for logging messages
    * using the UsbLogger class, which operates in a separate thread
    * and sends messages over USB CDC.
    * @param msg Pointer to the message string to be logged.
    * @retval None
    */
extern "C" void usb_logger_c_api(const char* msg) {
    UsbLogger::getInstance().log(msg);
}
/// @} // end of UsbLogger
/// @} // end of usb_logger_c_api
