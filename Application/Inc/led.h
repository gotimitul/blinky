/**
 * @file led.h
 * @brief LED control abstraction using GPIO pins
 * @version 1.0
 * @date 2025-08-07
 * @defgroup led LED Driver
 *
 * @{
 */

#ifndef __LED_H
#define __LED_H

#include "stdint.h"

#ifdef __cplusplus

/**
 * @class Led
 * @brief Base class for controlling an LED on a GPIO pin
 *
 * Provides the basic toggle functionality. Threading logic is implemented in
 * derived classes.
 */
class Led {
private:
public:
  ///< Default constructor
  Led() = default;
  // Static method to turn on an LED on a given GPIO pin
  static void on(uint32_t pin);
  // Static method to turn off an LED on a given GPIO pin
  static void off(uint32_t pin);
  // Static method to toggle an LED on a given GPIO pin
  static void toggle(uint32_t pin);
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */

/** @} */ // end of led
