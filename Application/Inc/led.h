/**
 * @file led.h
 * @brief LED control abstraction using GPIO pins
 * @author Mitul Goti
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
  static Led &getInstance() {
    static Led instance; // Guaranteed to be destroyed.
                         // Instantiated on first use.
    return instance;
  }
  ///< Default constructor
  Led() = default;

  void on(uint32_t pin);     /*!< Turn on LED */
  void off(uint32_t pin);    /*!< Turn off LED */
  void toggle(uint32_t pin); /*!< Toggle LED state */
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */

/** @} */ // end of led
