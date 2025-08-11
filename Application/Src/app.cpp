/**
 * @file app.cpp
 * @brief Application main function: initializes threads and synchronization
 *
 * @ingroup app
 *
 * This file contains the application entry point which initializes a shared semaphore
 * and starts multiple LED control threads with CMSIS-RTOS2.
 */

#include "app.h"
#include "led_thread.h"
#include "usb_logger.h"
#include "cmsis_os2.h" // Include CMSIS-RTOS2 header for RTOS functions

namespace
{
    constexpr uint32_t LED_BLUE_PIN = 63U;   ///< GPIO pin for blue LED
    constexpr uint32_t LED_RED_PIN = 62U;    ///< GPIO pin for red LED
    constexpr uint32_t LED_ORANGE_PIN = 61U; ///< GPIO pin for orange LED
    constexpr uint32_t LED_GREEN_PIN = 60U;  ///< GPIO pin for green LED
}

/**
 * @brief Main application thread entry
 *
 * This function creates a semaphore and launches four threads (blue, red, orange, green),
 * each controlling a different LED. Threads use the shared semaphore to avoid GPIO collisions.
 *
 * @param argument Unused (reserved for future extensions)
 */
void app_main(void *argument)
{
    UNUSED(argument); // CMSIS macro to mark unused variable
    // Initialize USB logger for debugging output
    UsbLogger::getInstance().init();
    // Semaphore for multiplexing access to GPIO pins
    osSemaphoreId_t semMultiplex = nullptr;
    // Create a semaphore for synchronizing access to GPIO pins
    semMultiplex = osSemaphoreNew(4, 1, nullptr);
    osDelay(5000); // Delay to allow USB CDC initialization
    // Check if semaphore was created successfully
    if (semMultiplex == nullptr)
    {
        UsbLogger::getInstance().log("Failed to create a semaphore at File: %s, Line: %d\r\n", __FILE__, __LINE__);
    }

    // Create static LED threads, one for each LED color
    static LedThread blue("blue", LED_BLUE_PIN, semMultiplex);
    static LedThread red("red", LED_RED_PIN, semMultiplex);
    static LedThread orange("orange", LED_ORANGE_PIN, semMultiplex);
    static LedThread green("green", LED_GREEN_PIN, semMultiplex);

    // Exit the application thread once all LED threads are launched
    osThreadExit();
}
