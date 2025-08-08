/**
 * @file led_thread.h
 * @brief Thread-enabled LED controller class
 *
 * @defgroup led_thread LED Thread
 * 
 * @{
 */

#ifndef __LEDTHREAD_H
#define __LEDTHREAD_H
/* includes --------------------------------------------------------------------------*/
#include "led.h"

#ifdef __cplusplus
extern "C" {
#endif

/* C headers includes -----------------------------------------------------------------*/
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
    uint32_t pin; ///< GPIO pin associated with this LED
    static void thread_entry(void* argument);

    osThreadId_t thread_id = NULL;      ///< CMSIS RTOS thread ID
    osSemaphoreId_t *sem;              ///< Shared semaphore pointer

    uint64_t stack[64] __attribute__((aligned(64))); ///< Static thread stack (aligned)
    uint64_t cb[32] __attribute__((aligned(64)));    ///< Static thread control block (aligned)

    osThreadAttr_t thread_attr;        ///< Thread attributes used by osThreadNew

    void start(void);				///< It creates a new thread
    void run(void);					///< It contains the control logic for an LED.

public:
	
    LedThread(const char* name, uint32_t pin, void* sem);
};

#endif /* __LEDTHREAD_H */

/** @} */ // end of led_thread
