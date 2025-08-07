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
#include "led.h"
#include "cmsis_os2.h"
#include "led_thread.h"
#include "usb_logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Main application thread entry
 *
 * This function creates a semaphore and launches four threads (blue, red, orange, green),
 * each controlling a different LED. Threads use the shared semaphore to avoid GPIO collisions.
 *
 * @param argument Unused (reserved for future extensions)
 */
void app_main(void *argument) {
    UNUSED(argument);  // CMSIS macro to mark unused variable

    // Create a counting semaphore with 4 tokens, 1 initially available
    osSemaphoreId_t semMultiplex;
    semMultiplex = osSemaphoreNew(4, 1, NULL);

    UsbLogger::getInstance().init();
    UsbLogger::getInstance().start();

    // Create static LED threads, one for each LED color
    static LedThread blue("blue", LED_BLUE_PIN, semMultiplex);
    static LedThread red("red", LED_RED_PIN, semMultiplex);
    static LedThread orange("orange", LED_ORANGE_PIN, semMultiplex);
    static LedThread green("green", LED_GREEN_PIN, semMultiplex);

    // Exit the application thread once all LED threads are launched
    osThreadExit();
}

#ifdef __cplusplus
}
#endif
