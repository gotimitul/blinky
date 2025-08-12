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
#include "usb_logger.h"

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
 *
 * This function creates a semaphore and launches four threads (blue, red,
 * orange, green), each controlling a different LED. Threads use the shared
 * semaphore to avoid GPIO collisions.
 *
 * @param argument Unused (reserved for future extensions)
 */
void app_main(void *argument) {
  UNUSED(argument); // CMSIS macro to mark unused variable

  Driver_GPIO0.Setup(
      USER_BUTTON_PIN,
      ARM_GPIO_SignalEvent); // Setup GPIO for user button with event callback
  Driver_GPIO0.SetEventTrigger(
      USER_BUTTON_PIN,
      ARM_GPIO_TRIGGER_RISING_EDGE); // Set event trigger for rising edge on
                                     // user button pin
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

void ARM_GPIO_SignalEvent(ARM_GPIO_Pin_t pin, uint32_t event) {
  // Signal the event to the LED thread
  osEventFlagsSet(app_events_get(), 1U); // Set the event flag for button press
}
