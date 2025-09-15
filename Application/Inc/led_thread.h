/**
 * @file led_thread.h
 * @brief Thread-enabled LED controller class
 * @author Mitul Goti
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
#include "cmsis_os2.h"
#include "led.h"
#include <cstdint>
#include <string_view>

#ifdef __cplusplus

#define LED_ON_TIME_MIN 100U  ///< Minimum LED on-time in ms
#define LED_ON_TIME_MAX 2000U ///< Maximum LED on-time in ms

/**
 * @class LedThread
 * @brief RTOS-threaded LED controller class
 *
 * Controls an LED using a dedicated RTOS thread. Provides static methods to
 * adjust the LED on-time within defined limits. Uses a semaphore for
 * synchronized access.
 */
class LedThread : public Led {
private:
  uint32_t pin; ///< GPIO pin associated with this LED
  static void thread_entry(void *argument);

  static uint32_t onTime;           ///< Delay time for LED ON state
  osThreadId_t thread_id = nullptr; ///< CMSIS RTOS thread ID
  osSemaphoreId_t sem;              ///< Shared semaphore pointer

  uint64_t stack[128]
      __attribute__((aligned(64))); ///< Static thread stack (aligned)
  uint64_t cb[32]
      __attribute__((aligned(64))); ///< Static thread control block (aligned)

  osThreadAttr_t thread_attr; ///< Thread attributes used by osThreadNew

  void checkButtonEvent(void *arg);
  void start(void); // It creates a new thread
  void run(void);   // It contains the control logic for an LED.

public:
  // Constructor initializes the LED pin and thread attributes
  LedThread(std::string_view threadName, uint32_t pin);
  ~LedThread() = default; // Default destructor

  osThreadId_t getThreadId(void) const { return thread_id; }

  inline static uint32_t getOnTime(void) { return onTime; } // Getter for onTime
  inline static void setOnTime(uint32_t t) { onTime = t; }  // Setter for onTime

  inline static void increaseOnTime(uint32_t delta) {
    onTime =
        (onTime + delta) > LED_ON_TIME_MAX ? LED_ON_TIME_MAX : (onTime + delta);
  } // Increase onTime
  inline static void decreaseOnTime(uint32_t delta) {
    onTime = (onTime > delta) ? (onTime - delta) : LED_ON_TIME_MAX;
  } // Decrease onTime with lower limit
};

osEventFlagsId_t app_events_get(void); // Get event flags for button press

constexpr uint32_t USER_BUTTON_FLAG = 0x00000001U
                                      << 0U; ///< Event flag for user button

extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LEDTHREAD_H */

/** @} */ // end of led_thread
