/**
 * @file led_thread.cpp
 * @brief Thread-based LED control using CMSIS-RTOS2
 *
 * Implements the LedThread class that manages LED blinking in its own thread.
 * Thread execution is synchronized using a shared semaphore to ensure mutual exclusion.
 */

#include "led_thread.h"
#include "led.h"

/**
 * @brief Construct a new LedThread object
 *
 * Initializes the LED pin via the base class and sets up the thread attributes for CMSIS-RTOS2.
 *
 * @param name Name of the thread (used by CMSIS-RTOS2 for debugging)
 * @param pin  GPIO pin number associated with the LED
 */
LedThread::LedThread(const char* name, uint32_t pin)
    : Led(pin)  // Call base class constructor to assign pin
{
    // Initialize thread attributes for CMSIS-RTOS2
    thread_attr = {
        .name = name,              // Thread name
        .attr_bits = 0U,           // No special thread attributes
        .cb_mem = cb,              // Memory for thread control block
        .cb_size = sizeof(cb),     // Size of control block
        .stack_mem = stack,        // Pointer to static stack
        .stack_size = sizeof(stack), // Stack size in bytes
        .priority = osPriorityNormal // Thread priority
    };
}

/**
 * @brief Start the thread to control LED blinking
 *
 * Spawns a new thread using CMSIS-RTOS2 and stores the shared semaphore reference.
 *
 * @param argument Pointer to a shared osSemaphoreId_t, passed to all threads
 */
void LedThread::start(void *argument) {
    // Start thread, passing 'this' so static entry can cast it back
    osThreadNew(thread_entry, this, &thread_attr);

    // Store semaphore pointer for thread-safe access to the LED
    this->sem = (osSemaphoreId_t*)argument;
}

/**
 * @brief Static entry point for the CMSIS-RTOS2 thread
 *
 * This function is required by CMSIS-RTOS2 and is used to invoke the actual
 * run method of the class instance.
 *
 * @param argument Pointer to the instance of LedThread
 */
void LedThread::thread_entry(void* argument) {
    // Cast back to LedThread and call the member function
    LedThread *thread = static_cast<LedThread*>(argument);
    thread->run();
}

/**
 * @brief Main thread function for blinking the LED
 *
 * This function continuously toggles the LED on and off with a delay.
 * It uses a shared semaphore to avoid simultaneous access to the same GPIO pins.
 */
void LedThread::run() {
    // Cast this pointer to base class to access toggle method
    Led *led = static_cast<Led*>(this);

    for (;;) {
        // Acquire semaphore before accessing the LED
        osSemaphoreAcquire(this->sem, osWaitForever);

        // Toggle LED ON
        led->toggle();
        osDelay(100); // Delay 100 ms

        // Toggle LED OFF
        led->toggle();

        // Release semaphore for next thread
        osSemaphoreRelease(this->sem);

        // Small delay to prevent aggressive rescheduling
        osDelay(1);
    }
}
