/**
 * @file app.cpp
 * @brief Application main function: initializes threads and synchronization
 * mechanisms
 * @author Mitul Goti
 * @version 1.1
 * @date 2025-09-13
 * @ingroup app
 *
 * @details
 * This file contains the application entry point which initializes a shared
 * semaphore, sets up GPIO for the user button, starts multiple LED control
 * threads, and launches a supervisor thread for runtime health monitoring.
 * Logging is routed via LogRouter to USB or file system as configured. All RTOS
 * objects use static allocation for reliability in embedded systems.
 *
 * # 📝 Overview
 * The Application Main module initializes the application by setting up GPIO
 * for the user button, creating LED control threads, initializing logging
 * backends, and launching a supervisor thread that monitors the health of all
 * LED and logger threads. The system uses CMSIS-RTOS2 for threading and
 * synchronization.
 *
 * # ⚙️ Features
 * - Initializes GPIO for user button with event callback.
 * - Creates multiple LED control threads (blue, red, orange, green).
 * - Initializes USB and file system loggers as configured.
 * - Supervisor thread monitors health of all threads and logs status/heartbeat.
 * - Uses CMSIS-RTOS2 for threading, synchronization, and static allocation.
 * - Handles button press events via GPIO interrupt and event flags.
 *
 * # 📋 Usage
 * The application starts by calling the `app_main` function, which sets up all
 * necessary components and enters an infinite loop. The supervisor thread runs
 * in the background, monitoring thread health and logging system status.
 *
 * # 🔧 Implementation Details
 * - The `app_main` function configures the GPIO pin for the user button to
 * trigger an event on a rising edge, initializes loggers, and creates LED
 * threads.
 * - Thread IDs are stored in an array for easy health monitoring.
 * - The supervisor thread checks the state of all threads and logs warnings if
 *   any are not running, as well as a periodic heartbeat.
 * - The GPIO event callback function `ARM_GPIO_SignalEvent` signals the LED
 * thread on button press using event flags (ISR-safe).
 * - All RTOS objects (threads, stacks, control blocks) use static allocation.
 * - The implementation is robust for concurrent logging and log replay over
 * USB.
 *
 * @version 1.1 - Added supervisor thread for runtime health monitoring and
 * improved documentation.
 */

#include "app.h"
#include "Driver_GPIO.h"
#include "cmsis_os2.h" // Include CMSIS-RTOS2 header for RTOS functions
#include "led_thread.h"
#include "log_router.h"
#include "stdio.h"
#include "stm32f4xx.h" // IWYU pragma: keep
#include "usb_logger.h"
#include <cstdint>

#ifdef FS_LOG
#include "fs_log.h"
#endif

extern ARM_DRIVER_GPIO Driver_GPIO0; // External GPIO driver instance
static void ARM_GPIO_SignalEvent(ARM_GPIO_Pin_t pin,
                                 uint32_t event); // GPIO event callback
static void supervisor_thread(void *argument);    // Supervisor thread function

namespace {
constexpr std::uint32_t USER_BUTTON_PIN = 0U; ///< GPIO pin for user button
constexpr std::uint32_t LED_BLUE_PIN = 63U;   ///< GPIO pin for blue LED
constexpr std::uint32_t LED_RED_PIN = 62U;    ///< GPIO pin for red LED
constexpr std::uint32_t LED_ORANGE_PIN = 61U; ///< GPIO pin for orange LED
constexpr std::uint32_t LED_GREEN_PIN = 60U;  ///< GPIO pin for green LED

osThreadId_t osThreadIds[5]; /*!< Array to hold thread IDs */

osThreadId_t supervisor_id;
uint64_t supervisor_stack[256]
    __attribute__((aligned(64))); /*!< Static thread stack (aligned) */
uint64_t supervisor_cb[32]
    __attribute__((aligned(64))); /*!< Static thread control block (aligned) */
osThreadAttr_t supervisor_attr = {
    .name = "supervisor",             /*!< Thread name */
    .attr_bits = 0U,                  /*!< No special thread attributes */
    .cb_mem = supervisor_cb,          /*!< Use static control block memory */
    .cb_size = sizeof(supervisor_cb), /*!< Use static control block size */
    .stack_mem = supervisor_stack,    /*!< Use static stack memory */
    .stack_size = sizeof(supervisor_stack), /*!< Stack size in bytes */
    .priority = osPriorityLow2,             /*!< Normal priority */
    .tz_module = 0U,                        /*!< Not used in this application */
};

} // namespace

/**
 * @brief Main application thread entry
 * This function initializes the GPIO for the user button, sets up event
 * triggers, initializes the USB logger, and creates LED threads.
 * This function is C-compatible and can be called from C code.
 * @param argument Unused (reserved for future extensions)
 */
extern "C" void app_main(void *argument) {
  UNUSED(argument); // CMSIS macro to mark unused variable

  // Setup GPIO for user button with event callback
  Driver_GPIO0.Setup(USER_BUTTON_PIN, ARM_GPIO_SignalEvent);
  // Set event trigger for rising edge on user button pin
  Driver_GPIO0.SetEventTrigger(USER_BUTTON_PIN, ARM_GPIO_TRIGGER_RISING_EDGE);
#ifdef RUN_TIME
  UsbLogger::getInstance().init(); // Initialize USB Logger for runtime logging
#endif
#if defined(FS_LOG) && !defined(DEBUG)
  FsLog::getInstance().init(); // Initialize File System Logger
#endif

  // Create static LED threads, one for each LED color
  static LedThread blue("blue", LED_BLUE_PIN);
  static LedThread red("red", LED_RED_PIN);
  static LedThread orange("orange", LED_ORANGE_PIN);
  static LedThread green("green", LED_GREEN_PIN);

  osThreadIds[0] = blue.getThreadId();   // Get thread ID for blue LED thread
  osThreadIds[1] = red.getThreadId();    // Get thread ID for red LED thread
  osThreadIds[2] = orange.getThreadId(); // Get thread ID for orange LED thread
  osThreadIds[3] = green.getThreadId();  // Get thread ID for green LED thread
  osThreadIds[4] =
      UsbLogger::getInstance().getThreadId(); // Reserved for supervisor thread

  // Create a supervisor thread to monitor LED threads
  supervisor_id = osThreadNew(supervisor_thread, nullptr, &supervisor_attr);

  if (supervisor_id == nullptr) {
#if defined(DEBUG) && !defined(FS_LOG)
    printf("Failed to create supervisor thread: %s, %d\r\n", __FILE__,
           __LINE__);
#elif RUN_TIME
    LogRouter::getInstance().log(
        "Program Fault: Failed to create supervisor thread\r\n");
#endif
  }

  while (1) {
    osDelay(1000U);
  }
  // Exit the application thread once all LED threads are launched
  osThreadExit();
}

/**
 * @brief   Supervisor thread to monitor LED threads
 * @details Monitors the health of LED threads and logs their status. If any
 * thread is found to be inactive, an error message is logged. The supervisor
 * also logs a heartbeat message every second if all threads are healthy.
 * @param   argument Unused (reserved for future extensions)
 */
static void supervisor_thread(void *argument) {
  UNUSED(argument);
  osThreadState_t state;
  std::string_view name;
  static std::atomic_uint8_t heartbeat = 0;
  while (1) {
    auto threadHealthCheck = [&]() {
      if (state == (osThreadInactive || osThreadError || osThreadTerminated)) {
#if defined(DEBUG) && !defined(FS_LOG)
        printf("%s thread not running!\r\n", name.data());
#endif
        LogRouter::getInstance().log("%s thread state is %d!\r\n", name.data(),
                                     state);
      }
    };

    for (size_t i = 0; i < sizeof(osThreadIds) / sizeof(osThreadIds[0]); i++) {
      if (osThreadIds[i] == nullptr) {
        continue;
      }
      state = osThreadGetState(osThreadIds[i]);
      name = osThreadGetName(osThreadIds[i]);
      threadHealthCheck();
    }
    heartbeat.fetch_add(1U); // Increment heartbeat counter
    LogRouter::getInstance().log("Supervisor: Heartbeat %d\r\n",
                                 heartbeat.load());
    osDelay(1000U); // Delay to reduce CPU usage
  }
}

/**
 * @brief Signal event callback for GPIO pin events
 * This function is called by the GPIO driver when a pin event occurs.
 * It signals the event to the LED thread to handle the button press.
 * @param pin GPIO pin that triggered the event
 * @param event Event type (e.g., rising edge)
 * @note This function is called from the GPIO driver interrupt context.
 */
static void ARM_GPIO_SignalEvent(ARM_GPIO_Pin_t pin, uint32_t event) {
  if (pin == USER_BUTTON_PIN && event == ARM_GPIO_EVENT_RISING_EDGE) {
    if (app_events_get() != nullptr) {
      // Signal the event to the LED thread
      uint32_t returnFlag = osEventFlagsSet(app_events_get(), USER_BUTTON_FLAG);
      if (returnFlag != USER_BUTTON_FLAG) {
#if defined(DEBUG) && !defined(FS_LOG)
        printf(
            "Failed to set event flag for button press: file: %s, line: %d\r\n",
            __FILE__, __LINE__);
#elif RUN_TIME
        LogRouter::getInstance().log(
            "Program Fault: Failed to set event flag for button press\r\n");
#endif
      }
    } else {
#if defined(DEBUG) && !defined(FS_LOG)
      printf("Failed to get event flags ID for button press: file: %s, line: "
             "%d\r\n",
             __FILE__, __LINE__);
#elif RUN_TIME
      LogRouter::getInstance().log(
          "Program Fault: Failed to get event flags ID for button press\r\n");
#endif
    }
  }
}
