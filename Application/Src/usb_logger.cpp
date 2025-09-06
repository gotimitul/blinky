/**
 * @file usb_logger.cpp
 * @brief Implementation of UsbLogger for USB CDC logging
 * @version 1.0
 * @date 2025-08-07
 * @ingroup UsbLogger
 */

#include "usb_logger.h"
#include "EventRecorder.h"
#include "boot_clock.h"
#include "cmsis_os2.h"
#include "led_thread.h"
#include "log_router.h"
#include "stdio.h" // For printf
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include <cstdint>
#include <cstring>
#include <stdint.h>
#include <string.h>

namespace {
constexpr uint32_t LOG_MSG_SIZE = 64;     // Size of each log message
constexpr uint32_t LOG_QUEUE_LENGTH = 32; // Number of messages in queue

osThreadId_t threadId = nullptr;         ///< RTOS thread ID for logger
osMessageQueueId_t msgQueueId = nullptr; ///< Message queue for log strings
osEventFlagsId_t usbXferFlag = nullptr;  ///< Event flags for USB transfer

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
    .name = "UsbLoggerQueue",         // Name for debugging
    .attr_bits = 0U,                  // No special attributes
    .cb_mem = log_queue_cb,           // No custom control block memory
    .cb_size = sizeof(log_queue_cb),  // Default size
    .mq_mem = log_queue_mem,          // Pointer to memory for queue
    .mq_size = sizeof(log_queue_mem), // Size of the memory buffer
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

/** @brief Private constructor for singleton pattern
 * Ensures only one instance of UsbLogger exists.
 */
UsbLogger::UsbLogger() {}

/** @brief Get the singleton instance of UsbLogger
 * This method returns a reference to the single instance of the UsbLogger
 * class, creating it if it does not already exist.
 */
UsbLogger &UsbLogger::getInstance() {
  static UsbLogger instance;
  return instance;
}

/** @brief Initialize the USB logger
 *
 * This method sets up the message queue and starts the logger thread.
 * It must be called before any logging can occur.
 */
void UsbLogger::init() {
  msgQueueId = osMessageQueueNew(LOG_QUEUE_LENGTH, LOG_MSG_SIZE, &msgQueueAttr);
  if (msgQueueId == nullptr) {
#ifdef DEBUG
    printf("Failed to create message queue for USB logger: %s, %d\r\n",
           __FILE__, __LINE__);
#endif
    return; // Safety check for uninitialized queue
  }
  threadId = osThreadNew(loggerThreadWrapper, this, &threadAttr);

  if (threadId == nullptr) {
#ifdef DEBUG
    printf("Failed to create USB logger thread: %s, %d\r\n", __FILE__,
           __LINE__);
#endif
    return; // Safety check for uninitialized thread
  }
  usbXferFlag = osEventFlagsNew(nullptr);
  if (usbXferFlag == nullptr) {
#ifdef DEBUG
    printf("Failed to create USB transfer event flags: %s, %d\r\n", __FILE__,
           __LINE__);
#endif
    return; // Safety check for uninitialized event flags
  }
  USBD_Interface_fops_FS.TransmitCplt = usbXferCompleteCallback;
}

/** @brief Static wrapper to call loggerThread from C-style function pointer.
 * @param argument Pointer to UsbLogger instance.
 */
void UsbLogger::loggerThreadWrapper(void *argument) {
  static_cast<UsbLogger *>(argument)->loggerThread();
}

/// Handler for message size errors
auto errorMsgTooBig = +[](void) {
#ifdef DEBUG
  printf("Warning: Message Size Exceeded. Last Message Truncated: %s, %d\r\n",
         __FILE__, __LINE__);
#endif
};

/** @brief Handler for full message queue
 * This function is called when the message queue is full and a new message
 * cannot be added. It removes the oldest message to make space for the new
 * message and logs a warning.
 */

auto messageQueueFullHandler = +[](void) {
  char logBuf[LOG_MSG_SIZE];
  osMessageQueueGet(msgQueueId, logBuf, 0, 0); // Try to clear the queue
#ifdef DEBUG
  printf("Warning: Message Queue Full. Last Message Removed: %s, %d\r\n",
         __FILE__, __LINE__);
#endif
};

/** @brief Log a simple message string.
 * @param msg The message string to log.
 */
void UsbLogger::log(const char *msg) {
  if (msgQueueId != nullptr && msg != nullptr) {
    while (osMessageQueuePut(msgQueueId, msg, 0, 0) ==
           osErrorResource) // non-blocking enqueue
    {
      messageQueueFullHandler();
    }
  }
}

/** @brief Main logger thread function
 *
 * This function runs in its own thread and continuously checks the message
 * queue for new log messages. When a message is available, it sends it over USB
 * CDC. It uses event flags to synchronize with the USB interrupt handler.
 */
void UsbLogger::loggerThread() {
  char logBuf[LOG_MSG_SIZE];
  bool usbXferCompleted = true; // Flag to track USB transfer completion

  for (;;) {
    osStatus_t status;
    // If previous USB transfer completed, get the next message from the queue
    if (usbXferCompleted == true) {
      status = osMessageQueueGet(msgQueueId, logBuf, nullptr, 100U);
    }
    if (status == osOK) {
      EventStartA(1); // Start event recording for USB transmission
      logBuf[LOG_MSG_SIZE - 1] = '\0'; // Ensure null-termination

      while (CDC_Transmit_FS(reinterpret_cast<uint8_t *>(logBuf),
                             strlen(logBuf)) != USBD_OK) {
        osDelay(10); // Wait for endpoint to be ready
      }
      osDelay(10); // Short delay to allow transfer to start
      // Wait for USB transfer flag to be set
      if (osEventFlagsWait(usbXferFlag, 1U, osFlagsWaitAny, 10U) != 1U) {
#ifdef DEBUG
        printf("Failed: USB transfer: %s, %d\r\n", __FILE__, __LINE__);
#endif
        usbXferCompleted = false; // Timeout or error occurred
      } else {
        usbXferCompleted = true; // Transfer completed successfully
      }
      EventStopA(1); // Stop event recording for USB transmission
    }
    loggerCommand(); // Check for incoming USB commands
  } // End of for loop
}

/** @brief Thread function that waits for commands and sends them via USB CDC.
 *
 * This function checks for incoming commands over USB CDC to adjust the LED
 * on-time. It reads a command string, parses it, and updates the on-time if
 * the command is valid.
 */
void UsbLogger::loggerCommand(void) {
  static char rxBuf[10];
  uint32_t rxLen = 4; // Length of received USB command

  if (USBD_Interface_fops_FS.Receive(reinterpret_cast<uint8_t *>(rxBuf),
                                     &rxLen) == USBD_OK) {
    uint32_t temp = 0;          // Temporary variable to hold parsed integer
    sscanf(rxBuf, "%u", &temp); // Parse integer from received buffer
    // If parsed value is within valid range, update onTime
    if (temp >= LED_ON_TIME_MIN && temp <= LED_ON_TIME_MAX) {
      LedThread::setOnTime(temp);
#ifdef RUN_TIME
      LogRouter::getInstance().log(
          "%s: Received USB command. New ON Time: %d ms\r\n",
          Time::getInstance().getCurrentTimeString(), LedThread::getOnTime());
#endif
    } else if (temp != 0) {
      LogRouter::getInstance().log(
          "Invalid ON Time received: %d. Enter between 100 and 2000.\r\n",
          temp);
#ifdef DEBUG
      printf("Invalid ON Time received: %s, %d\r\n", __FILE__, __LINE__);
#endif
    }
    std::memset(rxBuf, 0, 10); // Clear the receive buffer
  }
}

std::int32_t UsbLogger::usbXferChunk(const char *msg, uint32_t len) {
  if (msg != nullptr && len > 0) {
    while (CDC_Transmit_FS(reinterpret_cast<uint8_t *>(const_cast<char *>(msg)),
                           len) != USBD_OK) {
      osDelay(10); // Wait for endpoint to be ready
    }
    osDelay(10); // Short delay to allow transfer to start

    // Wait for USB transfer flag to be set
    if (osEventFlagsWait(usbXferFlag, 1U, osFlagsWaitAny, 10U) != 1U) {
#if defined(DEBUG) && !defined(FS_LOG)
      printf("Failed: USB chunk transfer: %s, %d\r\n", __FILE__, __LINE__);
#endif
      return -1; // Timeout or error occurred
    } else {
      return 0; // Transfer completed successfully
    }
  } else
    return -1; // Invalid parameters
}

/** @brief Checks if USB CDC is connected.
 * This function checks the USB device state to determine if it is configured
 * and ready for communication.
 * @return true if USB is connected, false otherwise.
 */
bool UsbLogger::usbIsConnected(void) {
  // This function can be used to check if USB is connected
  extern USBD_HandleTypeDef hUsbDeviceFS;
  if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
    // USB is connected and configured
    return true;
  } else {
    // USB is not connected
    return false;
  }
}

/* "C" type functions --------------------------------------------------------*/
extern "C" {
/** @brief Sets the USB transfer flag.
 *
 * This function is called from the USB CDC interrupt handler to signal that
 * data has been transmitted and the logger thread can proceed.
 */
auto usbXferFlagSet = []() {
  // Set the USB transfer flag to indicate data is ready to be sent
  if (usbXferFlag != nullptr) {
    uint32_t returnFlag = osEventFlagsSet(usbXferFlag, 1U);
    if (returnFlag != 1U) {
#ifdef DEBUG
      printf("Failed to set USB transfer event flag: file: %s, line: %d\r\n",
             __FILE__, __LINE__);
#endif
    }
  }
};

/** @brief USB transfer complete callback
 *
 * This function is called by the USB CDC driver when a data transfer is
 * complete. It sets the transfer flag to notify the logger thread.
 * @param Buf Pointer to the data buffer (unused)
 * @param Len Pointer to the length of data (unused)
 * @param epnum Endpoint number (unused)
 * @return USBD_OK on success
 */
int8_t usbXferCompleteCallback(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  int8_t result = USBD_OK;
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  // This function can be used to notify when USB transfer is complete
  usbXferFlagSet();
  return result;
}

/** * @brief C API function to log messages using UsbLogger.
 *
 * This function provides a C-style interface for logging messages
 * using the UsbLogger class, which operates in a separate thread
 * and sends messages over USB CDC.
 * @param msg Pointer to the message string to be logged.
 */
void usb_logger_c_api(const char *msg) {
  if (msg == nullptr) {
#ifdef DEBUG
    printf("usb_logger_c_api: msg is null: %s, %d\r\n", __FILE__, __LINE__);
#elif RUN_TIME
    LogRouter::getInstance().log("usb_logger_c_api: msg is null\r\n");
#endif
    return; // Safety check for null message
  }
  LogRouter::getInstance().log(msg);
}
} // extern "C"