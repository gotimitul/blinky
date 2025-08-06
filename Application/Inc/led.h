/**
 * @file led.h
 * @brief LED control abstraction using GPIO pins
 *
 * @defgroup led LED Driver
 * @ingroup led
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
    uint32_t pin; ///< GPIO pin associated with this LED

public:
    /**
     * @brief Constructor
     *
     * Initializes the LED pin.
     *
     * @param pin GPIO pin number
     */
    Led(uint32_t pin) : pin(pin) {}

    /**
     * @brief Toggle the state of the LED
     *
     * Reads the current GPIO input and sets the opposite output state.
     */
    void toggle(void);
};

#endif /* __LED_H */

/** @} */ // end of led
