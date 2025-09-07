/**
 * @file led_thread.cpp
 * @brief Thread-based LED control using CMSIS-RTOS2
 * @version 1.0
 * @date 2025-08-07
 * @ingroup led_thread
 *
 * @details
 *   Implements the LedThread class that manages LED blinking in its own thread.
 *   Thread execution is synchronized using a shared semaphore to ensure mutual
 *   exclusion. Provides event flag and semaphore mechanisms for inter-thread
 *   communication and safe GPIO access.
 */

#include "led_thread.h"
#include "boot_clock.h"
#include "cmsis_os2.h"
#include "led.h"
#include "log_router.h"
#include "stdio.h"
#include "string.h" // IWYU pragma: keep
#include <cstdint>
#include <cstring>
#include <mutex>

#ifdef DEBUG
#include "eventrecorder.h"
#endif

uint32_t LedThread::onTime = 500; /*!< Definition of static member variable */

/**
 * @namespace App
 * @brief Namespace for application events and synchronization mechanisms.
 * @details
 *   Contains functions and variables related to event flags and semaphores
 *   used for inter-thread communication in the application.
 */
namespace {

/**
 * @brief Static function to get a shared semaphore for LED threads.
 * @details
 *   Ensures that the semaphore is created only once and returns a pointer to
 * it. Uses a static once_flag to guarantee thread-safe initialization.
 * @return osSemaphoreId_t Pointer to the shared semaphore.
 */
osSemaphoreId_t shared_semaphore(void) {
  static std::once_flag
      sem_once_flag; /*!< Ensure semaphore is created only once */
  static osSemaphoreId_t semaphore = nullptr; /*!< Static semaphore pointer */
  std::call_once(sem_once_flag, []() {
    /* Create a semaphore with a maximum count of 1 and initial count of 1 */
    semaphore = osSemaphoreNew(1, 1, nullptr);
    if (semaphore == nullptr) {
#ifdef DEBUG
      printf("Failed to create shared semaphore: %s, %d\r\n", __FILE__,
             __LINE__);
#elif RUN_TIME
      LogRouter::getInstance().log("Failed to create shared semaphore\r\n");
#endif
    }
  });
  return semaphore;
}
} // namespace

/**
 * @brief Get the event flags for button press.
 * @details
 *   Initializes the event flags only once using std::call_once to ensure
 *   thread-safe access. Returns the event flags ID used for signaling button
 *   presses to the LED threads.
 * @return osEventFlagsId_t Event flags ID for button press.
 */
osEventFlagsId_t app_events_get() {
  static std::once_flag
      evt_once_flag; /*!< Ensure event flags are created only once */
  static osEventFlagsId_t evt_button =
      nullptr; /*!< Event flags for button press */
  std::call_once(evt_once_flag,
                 []() { evt_button = osEventFlagsNew(nullptr); });
  if (evt_button == nullptr) {
#ifdef DEBUG
    std::printf("Failed to create event flags for button press: %s, %d\r\n",
                __FILE__, __LINE__);
#elif RUN_TIME
    LogRouter::getInstance().log("Failed to create event flags for button "
                                 "press\r\n");
#endif
    return nullptr;
  }
  return evt_button;
}

/**
 * @brief Construct a new LedThread object.
 * @param threadName Name of the thread (used by CMSIS-RTOS2 for debugging).
 * @param pin  GPIO pin number associated with the LED.
 */
LedThread::LedThread(const char *threadName, uint32_t pin) : pin(pin) {
  /* Initialize thread attributes for CMSIS-RTOS2 */
  thread_attr = {
      .name = threadName,          /*!< Thread name */
      .attr_bits = 0U,             /*!< No special thread attributes */
      .cb_mem = cb,                /*!< Memory for thread control block */
      .cb_size = sizeof(cb),       /*!< Size of control block */
      .stack_mem = stack,          /*!< Pointer to static stack */
      .stack_size = sizeof(stack), /*!< Stack size in bytes */
      .priority = osPriorityNormal /*!< Thread priority */
  };
  /* Semaphore for multiplexing access to GPIO pins */
  this->sem = shared_semaphore();
  if (this->sem == nullptr) {
    return;
  }
  this->start();
}

/**
 * @brief Start the thread to control LED blinking.
 */
void LedThread::start(void) {
  /* Start thread, passing 'this' so static entry can cast it back */
  thread_id = osThreadNew(thread_entry, this, &thread_attr);
  if (thread_id == nullptr) {
#ifdef DEBUG
    printf("Failed to create LED thread %s. %s, %d\r\n", thread_attr.name,
           __FILE__, __LINE__);
#elif RUN_TIME
    LogRouter::getInstance().log("Failed to create LED thread %s\r\n",
                                 thread_attr.name);
#endif
    return;
  }
}

/**
 * @brief Static entry point for the CMSIS-RTOS2 thread.
 * @param argument Pointer to the instance of LedThread.
 */
void LedThread::thread_entry(void *argument) {
  if (argument == nullptr) {
#ifdef DEBUG
    printf("LedThread::thread_entry: argument is null: %s, %d\r\n", __FILE__,
           __LINE__);
#elif RUN_TIME
    LogRouter::getInstance().log(
        "LedThread::thread_entry: argument is null\r\n");
#endif
    osThreadExit();
  }
  LedThread *thread = static_cast<LedThread *>(argument);
  thread->run();
}

auto checkButtonEvent = [](void *arg) {
  LedThread *thread = static_cast<LedThread *>(arg);
  if (osEventFlagsWait(app_events_get(), 1U, osFlagsWaitAny, 0U) == 1U) {
    /*
    thread->decreaseOnTime(100U); // Decrease onTime by 100 ms
    #ifdef RUN_TIME
        LogRouter::getInstance().log("%s: Button pressed. New ON Time: %d
    ms\r\n", Time::getInstance().getCurrentTimeString(), thread->getOnTime());
    #endif
    */
    LogRouter::getInstance().replayFsLogsToUsb();
    osDelay(50U);                            /* Debounce delay */
    osEventFlagsClear(app_events_get(), 1U); /* Clear the event flag */
  }
};

/**
 * @brief Main control loop for the LED thread.
 * @details
 *   Contains the logic to toggle the LED on and off with a delay.
 *   Checks for button press events to adjust the on-time of the LED.
 *   Access to the LED GPIO pin is synchronized using a semaphore.
 */
void LedThread::run(void) {
#ifdef DEBUG
  const char *const str = osThreadGetName(thread_id);
  const char *const blue = "blue";
#endif
  for (;;) {
    /* Acquire semaphore before accessing the LED */
    osSemaphoreAcquire(sem, osWaitForever);
#ifdef DEBUG
    if (strcmp(str, blue) == 0)
      EventStartA(10);
#endif

    Led::getInstance().on(pin);

    LogRouter::getInstance().log("%s: LED %s ON for %d ms\r\n",
                                 Time::getInstance().getCurrentTimeString(),
                                 thread_attr.name, getOnTime());

    osDelay(getOnTime());

    Led::getInstance().off(pin);
#ifdef DEBUG
    if (strcmp(str, blue) == 0)
      EventStopA(10);
#endif
    /* Release semaphore for next thread */
    osSemaphoreRelease(sem);
    checkButtonEvent(this); /* Check for button press events */
    /* Small delay to prevent aggressive rescheduling */
    osThreadYield();
  }
}
