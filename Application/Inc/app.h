/**
 * @file app.h
 * @brief Application header file for RTOS-based LED control
 *
 * @defgroup app Application API
 *
 * @{
 */

#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h" // IWYU pragma: keep

void app_main(void *argument); ///< Main application thread function

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */

/** @} */ // end of app
