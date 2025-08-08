/**
 * @file led.h
 * @brief LED control abstraction using GPIO pins
 *
 * @defgroup led LED Driver
 * 
 * @{
 */

#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
}
#endif

/// GPIO pin definitions for different LED colors
#define LED_BLUE_PIN        63U
#define LED_RED_PIN         62U
#define LED_ORANGE_PIN      61U
#define LED_GREEN_PIN       60U

/**
 * @class Led
 * @brief Base class for controlling an LED on a GPIO pin
 *
 * Provides the basic toggle functionality. Threading logic is implemented in derived classes.
 */
class Led {
private:

public:
    Led() = default; ///< Default constructor
	
    void on(uint32_t pin); ///< Static method to turn on an LED on a given GPIO pin
    
    void off(uint32_t pin); ///< Static method to turn off an LED on a given GPIO pin

    void toggle(uint32_t pin) ; ///< Static method to toggle an LED on a given GPIO pin
};
#endif /* __LED_H */

/** @} */ // end of led
