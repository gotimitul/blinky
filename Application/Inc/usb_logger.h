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

    /**
     * @brief Get the singleton instance of UsbLogger.
     * @return Reference to the UsbLogger instance.
     */
    static UsbLogger& getInstance();

    /**
     * @brief Initialize the logger's internal message queue.
     */
    void init();

    /**
     * @brief Start the logger's background thread.
     */
    void start();

    /**
     * @brief Send a log message to be transmitted over USB CDC.
     *
     */
    void log(const char* msg);

    /// @}

private:
    /// @name Internal Mechanics
    /// @{

    UsbLogger();
    UsbLogger(const UsbLogger&) = delete;
    UsbLogger& operator=(const UsbLogger&) = delete;

    /**
     * @brief Thread wrapper used to call loggerThread().
     */
    static void loggerThreadWrapper(void *argument);

    /**
     * @brief Background thread for handling queued log messages.
     */
    void loggerThread();

    osThreadId_t threadId;         ///< RTOS thread ID for logger
    osMessageQueueId_t msgQueueId; ///< Message queue for log strings

    /// @}
};

#endif // USB_LOGGER_H

/** @} */ // end of UsbLogger group
