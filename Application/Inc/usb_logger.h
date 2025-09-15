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

#include <cmsis_os2.h>
#include <logger.h>
#include <stdint.h>
#include <string_view>

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
  static UsbLogger &getInstance();         /*!< Get singleton instance */
  void init();                             /*!< Initialize logger */
  void log(std::string_view msg) override; /*!< Log a message */
  UsbXferStatus usbXferChunk(std::string_view msg);     /*!< Send data chunk */
  osThreadId_t getThreadId() const { return threadId; } /*!< Get thread ID */

private:
  UsbLogger(); /*!< Private constructor for Singleton */
  UsbLogger(const UsbLogger &) = delete; /*!< Prevent copy construction */
  UsbLogger &operator=(const UsbLogger &) = delete; /*!< Prevent assignment */
  static void loggerThreadWrapper(void *argument);  /*!< Thread wrapper */
  UsbXferStatus usbXfer(std::string_view msg,
                        std::uint32_t len); /*!< Start USB transfer */
  void loggerThread();                      /*!< Logger thread function */
  void loggerCommand();                     /*!< Command handling function */

  osThreadId_t threadId = nullptr; /*!< RTOS thread ID for logger */

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