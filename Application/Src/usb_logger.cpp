/**
 * @file usb_logger.cpp
 * @brief Implementation of UsbLogger for USB CDC logging
 * @version 1.0
 * @date 2025-08-07
 * @ingroup UsbLogger
 */

/** @brief Include necessary headers for USB logging */
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

/** Anonymous namespace for internal linkage
 * @details Contains internal variables and helper functions for UsbLogger.
 */
namespace {
constexpr uint32_t LOG_MSG_SIZE = 64;     /*!< Size of each log message */
constexpr uint32_t LOG_QUEUE_LENGTH = 32; /*!< Number of messages in queue */

osThreadId_t threadId = nullptr;         /*!< RTOS thread ID for logger */
osMessageQueueId_t msgQueueId = nullptr; /*!< Message queue for log strings */
osEventFlagsId_t usbXferFlag = nullptr;  /*!< Event flags for USB transfer */

uint64_t log_queue_mem[LOG_QUEUE_LENGTH * LOG_MSG_SIZE / 8]
    __attribute__((aligned(64))); /*!< Memory buffer for message queue */
uint64_t log_queue_cb[32]
    __attribute__((aligned(64))); /*!< Control block for message queue */
uint64_t stack[256]
    __attribute__((aligned(64))); /*!< Static thread stack (aligned) */
uint64_t cb[32]
    __attribute__((aligned(64))); /*!< Static thread control block (aligned) */
constexpr osMessageQueueAttr_t msgQueueAttr = {
    .name = "UsbLoggerQueue",         /*!< Name for debugging */
    .attr_bits = 0U,                  /*!< No special attributes */
    .cb_mem = log_queue_cb,           /*!< Control block memory */
    .cb_size = sizeof(log_queue_cb),  /*!< Control block size */
    .mq_mem = log_queue_mem,          /*!< Pointer to memory for queue */
    .mq_size = sizeof(log_queue_mem), /*!< Size of the memory buffer */
};
constexpr osThreadAttr_t threadAttr = {
    .name = "UsbLoggerThread",   /*!< Name for debugging */
    .attr_bits = 0U,             /*!< No special thread attributes */
    .cb_mem = cb,                /*!< Memory for thread control block */
    .cb_size = sizeof(cb),       /*!< Size of control block */
    .stack_mem = stack,          /*!< Pointer to static stack */
    .stack_size = sizeof(stack), /*!< Stack size in bytes */
    .priority = osPriorityLow    /*!< Thread priority */
};
} // namespace

/**
 * @brief Private constructor for singleton pattern.
 * @details Only accessible by getInstance().
 */
UsbLogger::UsbLogger() {}

/**
 * @brief Get the singleton instance of UsbLogger.
 * @return Reference to the singleton UsbLogger instance.
 */
UsbLogger &UsbLogger::getInstance() {
  static UsbLogger instance;
  return instance;
}

/**
 * @brief Initialize the USB logger.
 * @details
 *   - Creates the message queue for log messages.
 *   - Starts the logger thread.
 *   - Initializes event flags for USB transfer completion.
 *   - Registers the USB transfer complete callback.
 */
void UsbLogger::init() {
  msgQueueId = osMessageQueueNew(LOG_QUEUE_LENGTH, LOG_MSG_SIZE, &msgQueueAttr);
  if (msgQueueId == nullptr) {
#ifdef DEBUG
    printf("Failed to create message queue for USB logger: %s, %d\r\n",
           __FILE__, __LINE__);
#endif
    return;
  }
  threadId = osThreadNew(loggerThreadWrapper, this, &threadAttr);

  if (threadId == nullptr) {
#ifdef DEBUG
    printf("Failed to create USB logger thread: %s, %d\r\n", __FILE__,
           __LINE__);
#endif
    return;
  }
  usbXferFlag = osEventFlagsNew(nullptr);
  if (usbXferFlag == nullptr) {
#ifdef DEBUG
    printf("Failed to create USB transfer event flags: %s, %d\r\n", __FILE__,
           __LINE__);
#endif
    return;
  }
  USBD_Interface_fops_FS.TransmitCplt = usbXferCompleteCallback;
}

/**
 * @brief Static wrapper to call loggerThread from C-style function pointer.
 * @param argument Pointer to UsbLogger instance.
 * @details Casts the argument and calls the member loggerThread().
 */
void UsbLogger::loggerThreadWrapper(void *argument) {
  static_cast<UsbLogger *>(argument)->loggerThread();
}

auto errorMsgTooBig = +[](void) {
#ifdef DEBUG
  printf("Warning: Message Size Exceeded. Last Message Truncated: %s, %d\r\n",
         __FILE__, __LINE__);
#endif
};

auto messageQueueFullHandler = +[](void) {
  char logBuf[LOG_MSG_SIZE];
  osMessageQueueGet(msgQueueId, logBuf, 0, 0); // Remove oldest message
#ifdef DEBUG
  printf("Warning: Message Queue Full. Last Message Removed: %s, %d\r\n",
         __FILE__, __LINE__);
#endif
};

/**
 * @brief Log a simple message string.
 * @param msg The message string to log.
 * @details
 *   - Puts the message into the logger's message queue.
 *   - If the queue is full, removes the oldest message and retries.
 */
void UsbLogger::log(const char *msg) {
  if (msgQueueId != nullptr && msg != nullptr) {
    while (osMessageQueuePut(msgQueueId, msg, 0, 0) ==
           osErrorResource) // non-blocking enqueue, remove oldest if full
    {
      messageQueueFullHandler();
    }
  }
}

/**
 * @brief Main logger thread function.
 * @details
 *   - Waits for messages in the queue.
 *   - Sends messages over USB CDC.
 *   - Waits for transfer completion event.
 *   - Handles command processing.
 */
void UsbLogger::loggerThread() {
  char logBuf[LOG_MSG_SIZE];
  bool usbXferCompleted = true;

  for (;;) {
    osStatus_t status;
    // Get next log message from queue if previous transfer completed
    if (usbXferCompleted == true) {
      status = osMessageQueueGet(msgQueueId, logBuf, nullptr, 100U);
    }
    // Check if a message was received
    if (status == osOK) {
      EventStartA(1);                  // Start event recording for profiling
      logBuf[LOG_MSG_SIZE - 1] = '\0'; // Ensure null termination

      // Transmit log message over USB CDC
      while (CDC_Transmit_FS(reinterpret_cast<uint8_t *>(logBuf),
                             strlen(logBuf)) != USBD_OK) {
        osDelay(10); // Wait and retry if USB is busy
      }
      osDelay(10);
      // Wait for transfer complete event
      if (osEventFlagsWait(usbXferFlag, 1U, osFlagsWaitAny, 10U) != 1U) {
#ifdef DEBUG
        printf("Failed: USB transfer: %s, %d\r\n", __FILE__, __LINE__);
#endif
        usbXferCompleted = false; // Retry sending next message
      } else {
        usbXferCompleted = true; // Transfer completed successfully
      }
      EventStopA(1); // Stop event recording
    }
    loggerCommand(); // Check for and process any incoming USB commands
  }
}

/**
 * @brief Thread function that waits for commands and sends them via USB CDC.
 * @details
 *   - Receives commands from USB CDC.
 *   - Parses and applies LED on-time commands.
 *   - Logs command results or errors.
 */
void UsbLogger::loggerCommand(void) {
  static char rxBuf[10];
  uint32_t rxLen = 4;
  // Check for received command from USB CDC
  if (USBD_Interface_fops_FS.Receive(reinterpret_cast<uint8_t *>(rxBuf),
                                     &rxLen) == USBD_OK) {
    uint32_t temp = 0;
    sscanf(rxBuf, "%u", &temp); // Parse received command as integer
    // Validate and apply LED on-time command
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
    std::memset(rxBuf, 0, 10); // Clear receive buffer
  }
}

/**
 * @brief Sends a chunk of data over USB CDC.
 * @param msg Pointer to the data buffer.
 * @param len Length of the data to send.
 * @return 0 on success, -1 on failure.
 * @details
 *   - Transmits the data chunk over USB CDC.
 *   - Waits for transfer completion event.
 */
std::int32_t UsbLogger::usbXferChunk(const char *msg, uint32_t len) {
  if (msg != nullptr && len > 0) {
    // Transmit a chunk of data over USB CDC
    while (CDC_Transmit_FS(reinterpret_cast<uint8_t *>(const_cast<char *>(msg)),
                           len) != USBD_OK) {
      osDelay(10); // Wait and retry if USB is busy
    }
    osDelay(10); // allow time for transfer to start

    // Wait for transfer complete event
    if (osEventFlagsWait(usbXferFlag, 1U, osFlagsWaitAny, 10U) != 1U) {
#if defined(DEBUG) && !defined(FS_LOG)
      printf("Failed: USB chunk transfer: %s, %d\r\n", __FILE__, __LINE__);
#endif
      return -1; // Transfer failed
    } else {
      return 0; // Transfer succeeded
    }
  } else
    return -1; // Invalid parameters
}

/**
 * @brief Checks if USB CDC is connected.
 * @return true if USB is connected, false otherwise.
 * @details Checks the USB device state for configuration.
 */
bool UsbLogger::usbIsConnected(void) {
  extern USBD_HandleTypeDef hUsbDeviceFS;
  if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
    return true; // USB is connected
  } else {
    return false; // USB is not connected
  }
}

/* "C" type functions --------------------------------------------------------*/
extern "C" {

/**
 * @brief Sets the USB transfer flag.
 * @details Sets the event flag to signal USB transfer completion.
 */
auto usbXferFlagSet = []() {
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

/**
 * @brief USB transfer complete callback.
 * @param Buf Pointer to the data buffer (unused)
 * @param Len Pointer to the length of data (unused)
 * @param epnum Endpoint number (unused)
 * @return USBD_OK on success
 * @details Called by the USB stack when a transfer is complete.
 */
int8_t usbXferCompleteCallback(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  int8_t result = USBD_OK;
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  usbXferFlagSet(); // Signal transfer completion
  return result;
}

/**
 * @brief C API function to log messages using UsbLogger.
 * @param msg Pointer to the message string to be logged.
 * @details Calls the LogRouter to log the message.
 */
void usb_logger_c_api(const char *msg) {
  if (msg == nullptr) {
#ifdef DEBUG
    printf("usb_logger_c_api: msg is null: %s, %d\r\n", __FILE__, __LINE__);
#elif RUN_TIME
    LogRouter::getInstance().log("usb_logger_c_api: msg is null\r\n");
#endif
    return;
  }
  LogRouter::getInstance().log(msg); // Route log message
}
} // extern "C"