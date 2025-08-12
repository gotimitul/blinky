/**
 * @file led_thread.cpp
 * @brief Thread-based LED control using CMSIS-RTOS2
 *
 * @ingroup led_thread
 *
 * Implements the LedThread class that manages LED blinking in its own thread.
 * Thread execution is synchronized using a shared semaphore to ensure mutual
 * exclusion.
 */

#include "led_thread.h"
#include "led.h"
#include "usb_logger.h"

/**
 * @brief Construct a new LedThread object
 *
 * Initializes the LED pin via the base class and sets up the thread attributes
 * for CMSIS-RTOS2.
 *
 * @param name Name of the thread (used by CMSIS-RTOS2 for debugging)
 * @param pin  GPIO pin number associated with the LED
 * @param sem  Semaphore reference that controls the execution of the threads
 */
LedThread::LedThread(const char *threadName, uint32_t pin, void *semaphore)
    : pin(pin), sem((osSemaphoreId_t *)
                        semaphore) // Call base class constructor to assign pin
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
    UsbLogger::getInstance().log("Failed to create LED thread %s\r\n",
                                 thread_attr.name);
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
    UsbLogger::getInstance().log(
        "LedThread::thread_entry: argument is null\r\n");
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
  if (sem == nullptr) {
    UsbLogger::getInstance().log(
        "Semaphore is null, cannot run LED thread\r\n");
    osThreadExit(); // Safety check for null semaphore
  }
  for (;;) {
    // Acquire semaphore before accessing the LED
    osSemaphoreAcquire(this->sem, osWaitForever);

    // Toggle LED ON
    Led::on(pin);
    osDelay(500); // Delay 500 ms

    UsbLogger::getInstance().log("LED %s is on\r\n",
                                 osThreadGetName(thread_id));

    // Toggle LED OFF
    Led::off(pin);

    // Release semaphore for next thread
    osSemaphoreRelease(this->sem);

    // Small delay to prevent aggressive rescheduling
    osDelay(1);
  }
}
