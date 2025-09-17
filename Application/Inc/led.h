/**
 * @file led.h
 * @brief LED control abstraction using GPIO pins
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @defgroup led LED_Driver
 *
 * @{
 */

#ifndef __LED_H
#define __LED_H

#include "stdint.h"
#include <cstdint>

#ifdef __cplusplus

/**
 * @class Led
 * @brief Base class for controlling an LED on a GPIO pin
 *
 * Provides the basic toggle functionality. Threading logic is implemented in
 * derived classes.
 */
class Led {
protected:
  enum class State : std::uint8_t { OFF = 0, ON = 1 };

private:
  std::uint32_t pin;        ///< GPIO pin associated with this LED
  State state = State::OFF; ///< Current state of the LED

public:
  Led(std::uint32_t pin, State state) : pin(pin), state(state) {
    if (state == State::ON) {
      on(pin);
    } else {
      off(pin);
    }
  }
  virtual ~Led() =
      default; ///< Virtual destructor for proper cleanup in derived classes

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
