/**
 * @file app.cpp
 * @brief Application main function: initializes threads and synchronization
 * mechanisms
 * @version 1.0
 * @date 2025-08-07
 * @ingroup app
 *
 * This file contains the application entry point which initializes a shared
 * semaphore and starts multiple LED control threads with CMSIS-RTOS2.
 */

#include "app.h"
#include "Driver_GPIO.h"
#include "cmsis_os2.h" // Include CMSIS-RTOS2 header for RTOS functions
#include "led_thread.h"
#include "stdio.h"
#include "usb_logger.h"
#include <cstdint>

extern ARM_DRIVER_GPIO Driver_GPIO0; // External GPIO driver instance
static void ARM_GPIO_SignalEvent(ARM_GPIO_Pin_t pin, uint32_t event);

namespace {
constexpr uint32_t USER_BUTTON_PIN = 0U; ///< GPIO pin for user button
constexpr uint32_t LED_BLUE_PIN = 63U;   ///< GPIO pin for blue LED
constexpr uint32_t LED_RED_PIN = 62U;    ///< GPIO pin for red LED
constexpr uint32_t LED_ORANGE_PIN = 61U; ///< GPIO pin for orange LED
constexpr uint32_t LED_GREEN_PIN = 60U;  ///< GPIO pin for green LED
} // namespace

/**
 * @brief Main application thread entry
 * This function initializes the GPIO for the user button, sets up event
 * triggers, initializes the USB logger, and creates LED threads.
 * @param argument Unused (reserved for future extensions)
 */
void app_main(void *argument) {
  UNUSED(argument); // CMSIS macro to mark unused variable

  // Setup GPIO for user button with event callback
  Driver_GPIO0.Setup(USER_BUTTON_PIN, ARM_GPIO_SignalEvent);
  // Set event trigger for rising edge on user button pin
  Driver_GPIO0.SetEventTrigger(USER_BUTTON_PIN, ARM_GPIO_TRIGGER_RISING_EDGE);
  // Initialize USB logger for debugging output
  UsbLogger::getInstance().init();

  // Create static LED threads, one for each LED color
  static LedThread blue("blue", LED_BLUE_PIN);
  static LedThread red("red", LED_RED_PIN);
  static LedThread orange("orange", LED_ORANGE_PIN);
  static LedThread green("green", LED_GREEN_PIN);

  // Exit the application thread once all LED threads are launched
  osThreadExit();
}

/**
 * @brief Signal event callback for GPIO pin events
 * This function is called by the GPIO driver when a pin event occurs.
 * It signals the event to the LED thread to handle the button press.
 * @param pin GPIO pin that triggered the event
 * @param event Event type (e.g., rising edge)
 * @note This function is called from the GPIO driver interrupt context.
 */
void ARM_GPIO_SignalEvent(ARM_GPIO_Pin_t pin, uint32_t event) {
  if (pin == USER_BUTTON_PIN && event == ARM_GPIO_EVENT_RISING_EDGE) {
    if (app_events_get() != nullptr) {
      // Signal the event to the LED thread
      uint32_t returnFlag = osEventFlagsSet(app_events_get(), 1U);
      if (returnFlag != 1U) {
#ifdef DEBUG
        printf(
            "Failed to set event flag for button press: file: %s, line: %d\r\n",
            __FILE__, __LINE__);
#elif RUN_TIME
        UsbLogger::getInstance().log(
            "Failed to set event flag for button press\r\n");
#endif
      }
    } else {
#ifdef DEBUG
      printf("Failed to get event flags ID for button press: file: %s, line: "
             "%d\r\n",
             __FILE__, __LINE__);
#elif RUN_TIME
      UsbLogger::getInstance().log(
          "Failed to get event flags ID for button press\r\n");
#endif
    }
  }
}
