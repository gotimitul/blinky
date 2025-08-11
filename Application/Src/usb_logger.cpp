/**
 * @file usb_logger.cpp
 * @brief Implementation of UsbLogger for USB CDC logging
 * @version 1.0
 * @date 2025-08-07
 * @ingroup UsbLogger
 */

#include "usb_logger.h"
#include <cstring>
#include "usbd_cdc_if.h"
#include "cstdarg" // For va_list, va_start, etc.

/// @addtogroup UsbLogger
/// @{

namespace
{
    constexpr osMessageQueueAttr_t msgQueueAttr = {
        .name = "UsbLoggerQueue", // Name for debugging
        .attr_bits = 0U,          // No special attributes
        .cb_mem = nullptr,        // No custom control block memory
        .cb_size = 0U,            // Default size
    };
    constexpr osThreadAttr_t threadAttr = {
        .name = "UsbLoggerThread", // Name for debugging
        .stack_size = 1024,        // Stack size in bytes
        .priority = osPriorityLow  // Thread priority
    };
    constexpr uint32_t LOG_MSG_SIZE = 128;    // Size of each log message
    constexpr uint32_t LOG_QUEUE_LENGTH = 10; // Number of messages in queue
}

/// @brief (Unused) memory buffer declared for alignment or extension
static uint8_t log_queue_mem[LOG_QUEUE_LENGTH * LOG_MSG_SIZE];

/// @brief Constructor initializes thread and queue handles to nullptr.
UsbLogger::UsbLogger() : threadId(nullptr), msgQueueId(nullptr) {}

/// @brief Returns reference to singleton logger instance.
UsbLogger &UsbLogger::getInstance()
{
    static UsbLogger instance;
    return instance;
}

/// @brief Initializes the logger's message queue.
void UsbLogger::init()
{
    msgQueueId = osMessageQueueNew(LOG_QUEUE_LENGTH, LOG_MSG_SIZE, &msgQueueAttr);
    if (msgQueueId == nullptr)
    {
        UsbLogger::getInstance().log("Message queue not initialized, cannot start logger thread\r\n");
        return; // Safety check for uninitialized queue
    }
    threadId = osThreadNew(loggerThreadWrapper, this, &threadAttr);
}

/// @brief Static wrapper to call loggerThread from C-style function pointer.
void UsbLogger::log(const char *msg)
{
    if (msgQueueId != nullptr && msg != nullptr)
    {
        osMessageQueuePut(msgQueueId, msg, 0, 0); // non-blocking enqueue
    }
}

/// @brief Log a message with a single integer value.
/// @param msg Format string for the log message.
/// @param val Integer value to include in the log message.
void UsbLogger::log(const char *msg, uint32_t val)
{
    if (msgQueueId != nullptr && msg != nullptr)
    {
        char logMsg[LOG_MSG_SIZE];
        snprintf(logMsg, LOG_MSG_SIZE, msg, val);
        osMessageQueuePut(msgQueueId, logMsg, 0, 0); // non-blocking enqueue
    }
}

/// @brief Log a message with a string value.
/// @param msg Format string for the log message.
/// @param str String value to include in the log message.
void UsbLogger::log(const char *msg, const char *str)
{
    if (msgQueueId != nullptr && msg != nullptr && str != nullptr)
    {
        char logMsg[LOG_MSG_SIZE];
        snprintf(logMsg, LOG_MSG_SIZE, msg, str);
        osMessageQueuePut(msgQueueId, logMsg, 0, 0); // non-blocking enqueue
    }
}

/// @brief Wrapper to start logger thread from C-style function pointer.
/// @param argument Pointer to UsbLogger instance.
void UsbLogger::loggerThreadWrapper(void *argument)
{
    static_cast<UsbLogger *>(argument)->loggerThread();
}

/// @brief Thread function that waits for messages and sends them via USB CDC.
void UsbLogger::loggerThread()
{
    char logBuf[LOG_MSG_SIZE];

    while (1)
    {
        if (osMessageQueueGet(msgQueueId, &logBuf, NULL, osWaitForever) == osOK)
        {
            while (CDC_Transmit_FS(reinterpret_cast<uint8_t *>(logBuf), strlen(logBuf)) == USBD_BUSY)
            {
                osDelay(1); // Wait for endpoint to be ready
            }
        }
    }
}

/// @}
