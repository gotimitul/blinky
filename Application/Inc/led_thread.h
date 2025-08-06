/**
 * @file led_thread.h
 * @brief Thread-enabled LED controller class
 *
 * Declares the `LedThread` class that runs a blinking LED logic inside a CMSIS-RTOS2 thread.
 */

#ifndef __LEDTHREAD_H
#define __LEDTHREAD_H

#include "led.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
}
#endif

/**
 * @class LedThread
 * @brief RTOS-threaded LED controller class
 *
 * Inherits from the `Led` class and runs an internal thread that toggles the LED.
 * Uses a semaphore for synchronized access to shared GPIO lines.
 */
class LedThread : public Led {
private:
    /**
     * @brief Static RTOS thread entry point
     *
     * Casts the argument back to the `LedThread` object and calls its `run()` method.
     *
     * @param argument Pointer to `LedThread` instance
     */
    static void thread_entry(void* argument);

    /**
     * @brief Internal method to execute the thread loop
     *
     * Toggles the LED and synchronizes using a semaphore.
     */
    void run();

    osThreadId_t thread_id = NULL;      ///< CMSIS RTOS thread ID
    osSemaphoreId_t *sem;              ///< Shared semaphore pointer

    uint64_t stack[64] __attribute__((aligned(64))); ///< Static thread stack (aligned)
    uint64_t cb[32] __attribute__((aligned(64)));    ///< Static thread control block (aligned)

    osThreadAttr_t thread_attr;        ///< Thread attributes used by osThreadNew

public:
    /**
     * @brief Constructor
     *
     * Initializes the LED pin and sets up thread attributes.
     *
     * @param name Thread name
     * @param pin GPIO pin number for the LED
     */
    LedThread(const char* name, uint32_t pin);

    /**
     * @brief Starts the thread execution
     *
     * @param argument Pointer to the shared semaphore
     */
    void start(void *argument);
};

#endif /* __LEDTHREAD_H */
