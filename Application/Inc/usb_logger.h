/**
 * @file usb_logger.h
 * @brief UsbLogger class for USB CDC-based logging (CMSIS-RTOS2)
 * @version 1.0
 * @date 2025-08-07
 * @author  Mitul Goti
 * @ingroup Logger
 * @{
 * @details This file provides the interface for the UsbLogger class,
 *          which is responsible for logging messages to a USB CDC interface.
 */

#ifndef USB_LOGGER_H
#define USB_LOGGER_H

#include <logger.h>
#include <stdint.h>

#ifdef __cplusplus

/**
 * @class   UsbLogger
 * @brief   Singleton class for USB CDC logging.
 * @details
 *   Provides methods for initializing the USB logger, logging messages, and
 *   handling USB communication.
 *   Thread safety is ensured using RTOS primitives.
 */
class UsbLogger : public Logger {
public:
  static UsbLogger &getInstance();    /*!< Get singleton instance */
  void init();                        /*!< Initialize logger */
  void log(const char *msg) override; /*!< Log a message */
  std::int32_t usbXferChunk(const char *msg,
                            uint32_t len); /*!< Send data chunk */

private:
  UsbLogger(); /*!< Private constructor for Singleton */
  UsbLogger(const UsbLogger &) = delete; /*!< Prevent copy construction */
  UsbLogger &operator=(const UsbLogger &) = delete; /*!< Prevent assignment */
  static void loggerThreadWrapper(void *argument);  /*!< Thread wrapper */
  void loggerThread();  /*!< Logger thread function */
  void loggerCommand(); /*!< Command handling function */

  /** @brief Check if USB is connected */
  bool usbIsConnected(void); /*!< Check if USB is connected */
};

extern "C" {
#endif

int8_t
usbXferCompleteCallback(uint8_t *Buf, uint32_t *Len,
                        uint8_t epnum); /*!< USB transfer complete callback */
void usb_logger_c_api(const char *msg); /*!< C API for logging */

#ifdef __cplusplus
}
#endif

#endif // USB_LOGGER_H

/** @} */ // end of Logger