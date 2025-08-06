/**
 * @file app.h
 * @brief Application header file for RTOS-based LED control
 *
 * Declares the main application thread entry and argument structure.
 */

#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * @struct argument_t
 * @brief Struct for passing thread configuration parameters
 *
 * Used to encapsulate runtime configuration such as LED pin and delay time.
 */
typedef struct 
{
    uint32_t delay; ///< Delay time in milliseconds
    uint32_t pin;   ///< GPIO pin number
} argument_t;

/**
 * @brief Entry point for the main application logic
 *
 * This function is called after the RTOS kernel is initialized and started.
 * It initializes synchronization primitives and launches LED threads.
 *
 * @param argument Optional pointer to arguments (currently unused)
 */
void app_main(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */
