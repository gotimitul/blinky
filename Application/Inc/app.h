/**
 * @file app.h
 * @brief Application header file for RTOS-based LED control
 *
 * @defgroup app Application API
 * @ingroup app
 * @{
 */

#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * @brief Struct for passing thread configuration parameters
 */
typedef struct 
{
    uint32_t delay; ///< Delay time in milliseconds
    uint32_t pin;   ///< GPIO pin number
} argument_t;

/**
 * @brief Entry point for the main application logic
 * @param argument Optional pointer to arguments (currently unused)
 */
void app_main(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */

/** @} */ // end of app
