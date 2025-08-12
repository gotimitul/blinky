/**
 * @file led_thread.h
 * @brief Thread-enabled LED controller class
 * @version 1.0
 * @date 2025-08-07
 * @defgroup led_thread LED Thread
 *
 * @{
 */

#ifndef __LEDTHREAD_H
#define __LEDTHREAD_H
/* includes
 * --------------------------------------------------------------------------*/
#include "led.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* C headers includes
 * -----------------------------------------------------------------*/
#include "cmsis_os2.h"

#ifdef __cplusplus
}
#endif

/**
 * @class LedThread
 * @brief RTOS-threaded LED controller class
 *
 * Inherits from the `Led` class and runs an internal thread that toggles the
 * LED. Uses a semaphore for synchronized access to shared GPIO lines.
 */
class LedThread : public Led {
private:
  uint32_t pin; ///< GPIO pin associated with this LED
  static void thread_entry(void *argument);
  osSemaphoreId_t shared_semaphore(void);

  osThreadId_t thread_id = NULL; ///< CMSIS RTOS thread ID
  osSemaphoreId_t sem;           ///< Shared semaphore pointer

  uint64_t stack[64]
      __attribute__((aligned(64))); ///< Static thread stack (aligned)
  uint64_t cb[32]
      __attribute__((aligned(64))); ///< Static thread control block (aligned)

  osThreadAttr_t thread_attr; ///< Thread attributes used by osThreadNew

  void start(void); // It creates a new thread
  void run(void);   // It contains the control logic for an LED.

public:
  // Constructor initializes the LED pin and thread attributes
  LedThread(const char *threadName, uint32_t pin);
};

osEventFlagsId_t app_events_get(void); // Get event flags for button press

#endif /* __LEDTHREAD_H */

/** @} */ // end of led_thread
