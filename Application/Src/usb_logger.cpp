/**
 * @file usb_logger.cpp
 * @brief Implementation of UsbLogger for USB CDC logging
 * @version 1.0
 * @date 2025-08-07
 * @ingroup UsbLogger
 */

#include "usb_logger.h"
#include "cstdarg" // For va_list, va_start, etc.
#include "usbd_cdc_if.h"
#include <cstdint>
#include <cstring>

namespace {
constexpr uint32_t LOG_MSG_SIZE = 128;    // Size of each log message
constexpr uint32_t LOG_QUEUE_LENGTH = 10; // Number of messages in queue

osThreadId_t threadId = nullptr;         ///< RTOS thread ID for logger
osMessageQueueId_t msgQueueId = nullptr; ///< Message queue for log strings
/// @brief (Unused) memory buffer declared for alignment or extension
uint64_t log_queue_mem[LOG_QUEUE_LENGTH * LOG_MSG_SIZE / 8]
    __attribute__((aligned(64))); ///< Memory buffer for message queue
uint64_t log_queue_cb[32]
    __attribute__((aligned(64))); ///< Control block for message queue
uint64_t stack[256]
    __attribute__((aligned(64))); ///< Static thread stack (aligned)
uint64_t cb[32]
    __attribute__((aligned(64))); ///< Static thread control block (aligned)
constexpr osMessageQueueAttr_t msgQueueAttr = {
    .name = "UsbLoggerQueue", // Name for debugging
    .attr_bits = 0U,          // No special attributes
    .cb_mem = log_queue_cb,        // No custom control block memory
    .cb_size = sizeof(log_queue_cb),            // Default size
    .mq_mem = log_queue_mem,        // Pointer to memory for queue
    .mq_size = sizeof(log_queue_mem),            // Size of the memory buffer
};
constexpr osThreadAttr_t threadAttr = {
    .name = "UsbLoggerThread",   // Name for debugging
    .attr_bits = 0U,             // No special thread attributes
    .cb_mem = cb,                // Memory for thread control block
    .cb_size = sizeof(cb),       // Size of control block
    .stack_mem = stack,          // Pointer to static stack
    .stack_size = sizeof(stack), // Stack size in bytes
    .priority = osPriorityLow    // Thread priority
};
} // namespace

/// @brief Constructor initializes thread and queue handles to nullptr.
UsbLogger::UsbLogger() {}

/// @brief Returns reference to singleton logger instance.
UsbLogger &UsbLogger::getInstance() {
  static UsbLogger instance;
  return instance;
}

/// @brief Initializes the logger's message queue.
void UsbLogger::init() {
  msgQueueId = osMessageQueueNew(LOG_QUEUE_LENGTH, LOG_MSG_SIZE, &msgQueueAttr);
  if (msgQueueId == nullptr) {
    return; // Safety check for uninitialized queue
  }
  threadId = osThreadNew(loggerThreadWrapper, this, &threadAttr);

#ifdef USE_FULL_ASSERT
  if (threadId == nullptr) {
    assert_param(threadId != nullptr);
    return; // Safety check for uninitialized thread
  }
#endif
}

/// @brief Wrapper to start logger thread from C-style function pointer.
/// @param argument Pointer to UsbLogger instance.
void UsbLogger::loggerThreadWrapper(void *argument) {
  static_cast<UsbLogger *>(argument)->loggerThread();
}

/// @brief Static wrapper to call loggerThread from C-style function pointer.
void UsbLogger::log(const char *msg) {
  if (msgQueueId != nullptr && msg != nullptr) {
    char logMsg[LOG_MSG_SIZE];
    snprintf(logMsg, LOG_MSG_SIZE, "%s", msg);
    osMessageQueuePut(msgQueueId, logMsg, 0, 0); // non-blocking enqueue
  }
}

/// @brief Log a message with a single integer value.
/// @param msg Format string for the log message.
/// @param val Integer value to include in the log message.
void UsbLogger::log(const char *msg, uint32_t val) {
  if (msgQueueId != nullptr && msg != nullptr) {
    char logMsg[LOG_MSG_SIZE];
    snprintf(logMsg, LOG_MSG_SIZE, msg, val);
    osMessageQueuePut(msgQueueId, logMsg, 0, 0); // non-blocking enqueue
  }
}

/// @brief Log a message with a string value.
/// @param msg Format string for the log message.
/// @param str String value to include in the log message.
void UsbLogger::log(const char *msg, const char *str) {
  if (msgQueueId != nullptr && msg != nullptr && str != nullptr) {
    char logMsg[LOG_MSG_SIZE];
    snprintf(logMsg, LOG_MSG_SIZE, msg, str);
    osMessageQueuePut(msgQueueId, logMsg, 0, 0); // non-blocking enqueue
  }
}

void UsbLogger::log(const char *msg, const char *str, uint32_t val) {
  if (msgQueueId != nullptr && msg != nullptr && str != nullptr) {
    char logMsg[LOG_MSG_SIZE];
    snprintf(logMsg, LOG_MSG_SIZE, msg, str, val);
    osMessageQueuePut(msgQueueId, logMsg, 0, 0); // non-blocking enqueue
  }
}

/// @brief Thread function that waits for messages and sends them via USB CDC.
void UsbLogger::loggerThread() {
  char logBuf[LOG_MSG_SIZE];

  for (;;) {
    if (osMessageQueueGet(msgQueueId, logBuf, NULL, osWaitForever) == osOK) {
      logBuf[LOG_MSG_SIZE - 1] = '\0'; // Ensure null-termination
      while (CDC_Transmit_FS(reinterpret_cast<uint8_t *>(logBuf),
                             strlen(logBuf)) == USBD_BUSY) {
        osDelay(1); // Wait for endpoint to be ready
      }
    }
  }
}
