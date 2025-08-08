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
#include  "cstdarg" // For va_list, va_start, etc.

/// @addtogroup UsbLogger
/// @{

/// @brief Maximum size of each log message (bytes)
#define LOG_MSG_SIZE     128

/// @brief Number of messages that can be stored in queue
#define LOG_QUEUE_LENGTH 10

/// @brief (Unused) memory buffer declared for alignment or extension
static uint8_t log_queue_mem[LOG_QUEUE_LENGTH * LOG_MSG_SIZE];

/// @brief Constructor initializes thread and queue handles to nullptr.
UsbLogger::UsbLogger() : threadId(nullptr), msgQueueId(nullptr) {}

/// @brief Returns reference to singleton logger instance.
UsbLogger& UsbLogger::getInstance() {
    static UsbLogger instance;
    return instance;
}

/// @brief Initializes the logger's message queue.
void UsbLogger::init() {
    msgQueueId = osMessageQueueNew(LOG_QUEUE_LENGTH, LOG_MSG_SIZE, nullptr);
}

/// @brief Starts the logger's background thread.
void UsbLogger::start() {
    const osThreadAttr_t attr = {
        .name = "UsbLoggerThread",
        .stack_size = 1024,
        .priority = osPriorityLow
    };
    threadId = osThreadNew(loggerThreadWrapper, this, &attr);
}

/// @brief Queues a log message for USB transmission.
/// @param msg Null-terminated string (max 127 bytes)
void UsbLogger::log(const char* format, ...) {
    if (msgQueueId != nullptr && format != nullptr) {
        char msg[LOG_MSG_SIZE];
        va_list args;
        va_start(args, format);
        vsnprintf(msg, LOG_MSG_SIZE, format, args);
        va_end(args);
        osMessageQueuePut(msgQueueId, msg, 0, 0); // non-blocking enqueue
    }
}

/// @brief Wrapper to start logger thread from C-style function pointer.
/// @param argument Pointer to UsbLogger instance.
void UsbLogger::loggerThreadWrapper(void *argument) {
    static_cast<UsbLogger*>(argument)->loggerThread();
}

/// @brief Thread function that waits for messages and sends them via USB CDC.
void UsbLogger::loggerThread() {
    char logBuf[LOG_MSG_SIZE];

    while (1) {
        if (osMessageQueueGet(msgQueueId, &logBuf, NULL, osWaitForever) == osOK) {
            while (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(logBuf), strlen(logBuf)) == USBD_BUSY) {
                osDelay(1); // Wait for endpoint to be ready
            }
        }
    }
}

/// @}
