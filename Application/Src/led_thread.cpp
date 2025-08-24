/**
 * @file led_thread.cpp
 * @brief Thread-based LED control using CMSIS-RTOS2
 * @version 1.0
 * @date 2025-08-07
 * @ingroup led_thread
 *
 * Implements the LedThread class that manages LED blinking in its own thread.
 * Thread execution is synchronized using a shared semaphore to ensure mutual
 * exclusion.
 */

#include "led_thread.h"
#include "cmsis_os2.h"
#include "eventrecorder.h"
#include "led.h"
#include "stdio.h"
#include "string.h"
#include "usb_logger.h"
#include <cstdint>
#include <mutex>

/** * @namespace
 * @brief Namespace for application events and synchronization mechanisms
 * This namespace contains functions and variables related to event flags and
 * semaphores used for inter-thread communication in the application.
 */
namespace {
uint32_t onTime = 500U; ///< Default delay for LED toggling
uint32_t counter = 0U;  ///< Counter for LED toggling
/** * @brief Static function to get a shared semaphore for LED threads
 *
 * This function ensures that the semaphore is created only once and returns a
 * pointer to it. It uses a static once_flag to guarantee thread-safe
 * initialization.
 * @return osSemaphoreId_t Pointer to the shared semaphore
 */
osSemaphoreId_t shared_semaphore(void) {
  static std::once_flag sem_once_flag; // Ensure semaphore is created only once
  static osSemaphoreId_t semaphore = nullptr; // Static semaphore pointer
  std::call_once(sem_once_flag, []() {
    // Create a semaphore with a maximum count of 1 and initial count of 1
    semaphore = osSemaphoreNew(1, 1, nullptr);
    if (semaphore == nullptr) {
#ifdef DEBUG
      printf("Failed to create shared semaphore: %s, %d\r\n", __FILE__,
             __LINE__);
#elif RUN_TIME
      UsbLogger::getInstance().log("Failed to create shared semaphore\r\n");
#endif
    }
  });
  return semaphore;
}
} // namespace

/** * @brief Get the event flags for button press
 *
 * This function initializes the event flags only once using std::call_once to
 * ensure thread-safe access. It returns the event flags ID used for signaling
 * button presses to the LED threads.
 * @return osEventFlagsId_t Event flags ID for button press
 */
osEventFlagsId_t app_events_get() {
  ///< Ensure event flags are created only once
  static std::once_flag evt_once_flag;
  ///< Event flags for button press
  static osEventFlagsId_t evt_button = nullptr;
  // Create event flags only once using std::call_once
  std::call_once(evt_once_flag,
                 []() { evt_button = osEventFlagsNew(nullptr); });
  // Check if event flags were created successfully
  if (evt_button == nullptr) {
#ifdef DEBUG
    std::printf("Failed to create event flags for button press: %s, %d\r\n",
                __FILE__, __LINE__);
#elif RUN_TIME
    UsbLogger::getInstance().log("Failed to create event flags for button "
                                 "press\r\n");
#endif
    return nullptr; // Return null if creation failed
  }
  return evt_button; // Return the event flags ID
}

/**
 * @brief Construct a new LedThread object
 *
 * Initializes the LED pin via the base class and sets up the thread attributes
 * for CMSIS-RTOS2.
 *
 * @param threadName Name of the thread (used by CMSIS-RTOS2 for debugging)
 * @param pin  GPIO pin number associated with the LED
 * @param semaphore  Semaphore reference that controls the execution of the
 * threads
 */
LedThread::LedThread(const char *threadName, uint32_t pin)
    : pin(pin) // Call base class constructor to assign pin
{
  // Initialize thread attributes for CMSIS-RTOS2
  thread_attr = {
      .name = threadName,          // Thread name
      .attr_bits = 0U,             // No special thread attributes
      .cb_mem = cb,                // Memory for thread control block
      .cb_size = sizeof(cb),       // Size of control block
      .stack_mem = stack,          // Pointer to static stack
      .stack_size = sizeof(stack), // Stack size in bytes
      .priority = osPriorityNormal // Thread priority
  };
  // Semaphore for multiplexing access to GPIO pins
  this->sem = shared_semaphore(); // Get the shared semaphore
  if (this->sem == nullptr) {
    return; // If semaphore creation failed, exit constructor
  }
  this->start();
}

/**
 * @brief Start the thread to control LED blinking
 *
 * Spawns a new thread using CMSIS-RTOS2 and stores the shared semaphore
 * reference.
 *
 */
void LedThread::start(void) {
  // Start thread, passing 'this' so static entry can cast it back
  thread_id = osThreadNew(thread_entry, this, &thread_attr);
  if (thread_id == nullptr) {
#ifdef DEBUG
    printf("Failed to create LED thread %s. %s, %d\r\n", thread_attr.name,
           __FILE__, __LINE__);
#elif RUN_TIME
    UsbLogger::getInstance().log("Failed to create LED thread %s\r\n",
                                 thread_attr.name);
#endif
    return; // If thread creation failed, exit start method
  }
}

/**
 * @brief Static entry point for the CMSIS-RTOS2 thread
 *
 * This function is required by CMSIS-RTOS2 and is used to invoke the actual
 * run method of the class instance.
 *
 * @param argument Pointer to the instance of LedThread
 */
void LedThread::thread_entry(void *argument) {
  // Safety check for null argument
  if (argument == nullptr) {
#ifdef DEBUG
    printf("LedThread::thread_entry: argument is null: %s, %d\r\n", __FILE__,
           __LINE__);
#elif RUN_TIME
    UsbLogger::getInstance().log(
        "LedThread::thread_entry: argument is null\r\n");
#endif
    osThreadExit();
  }
  // Cast back to LedThread and call the member function
  LedThread *thread = static_cast<LedThread *>(argument);
  thread->run();
}

/**
 * @brief Main thread function for blinking the LED
 *
 * This function continuously toggles the LED on and off with a delay.
 * It uses a shared semaphore to avoid simultaneous access to the same GPIO
 * pins.
 */
void LedThread::run(void) {
  //  extern osEventFlagsId_t evt_button;

  for (;;) {
    const char *str = osThreadGetName(thread_id);
    const char *blue = "blue";
    uint32_t debounceTime = 0u; // Debounce time in milliseconds
    // Acquire semaphore before accessing the LED
    osSemaphoreAcquire(sem, osWaitForever);
    if (strcmp(str, blue) == 0)
      EventStartA(10);
    // Check if the event flag for button press is set
    // If the button is pressed, adjust the onTime delay
    // and clear the event flag
    if ((osEventFlagsGet(app_events_get()) & 1U) == 1U) {
      onTime = onTime > 100U ? onTime - 100U : 1000U;
      Led::on(pin);          // Turn on the LED immediately
      debounceTime = 50U;    // Set debounce time to 50 ms
      osDelay(debounceTime); // Debounce delay
      osEventFlagsClear(app_events_get(), 1U);
#ifdef RUN_TIME
      UsbLogger::getInstance().log("Button pressed. New onTime: %d ms\r\n",
                                   onTime);
#endif
    }
    // Toggle LED ON
    Led::on(pin);
#ifdef RUN_TIME
//    UsbLogger::getInstance().log("LED %s is on: %d\r\n",
//                                 osThreadGetName(thread_id), counter++);
#endif
    osDelay(onTime - debounceTime); // Delay for the specified time

    // Toggle LED OFF
    Led::off(pin);
    if (strcmp(str, blue) == 0)
      EventStopA(10);
    // Release semaphore for next thread
    osSemaphoreRelease(sem);

    // Small delay to prevent aggressive rescheduling
    osDelay(1);
  }
}
