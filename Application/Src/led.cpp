/**
 * @file led.cpp
 * @brief Implementation of the Led class for GPIO-based LED control
 * @author Mitul Goti
 * @version 1.0
 * @date 2025-08-07
 * @ingroup led
 * @details
 * This file defines the toggle method of the Led class, using the
 * CMSIS-compliant ARM GPIO driver to change the LED state based on its current
 * input.
 */

/* LED Control
 ---
 This module provides an interface for controlling LEDs connected to GPIO pins.

 # 📝 Overview
 The LED control module allows for easy manipulation of LED states
 (on/off/toggle) through a simple API. It abstracts the underlying GPIO
 operations, providing a higher-level interface for LED control.

 # ⚙️ Features
 - Turn LEDs on and off.
 - Toggle LED states.

 # 📋 Usage
 To use the LED control module, create an instance of the `Led` class and call
 the desired methods (on, off, toggle) with the appropriate GPIO pin number.

 # 🔧 Implementation Details
 The `Led` class uses the CMSIS-compliant `ARM_DRIVER_GPIO` interface to
 manipulate GPIO pins. The `toggle` method reads the current state of the pin
 and sets it to the opposite state.

 The implementation ensures that the GPIO driver is properly initialized and
 configured before performing any operations.
*/

#include "led.h"
#include "Driver_GPIO.h" // CMSIS-Driver GPIO interface

extern ARM_DRIVER_GPIO Driver_GPIO0; // External GPIO driver instance

/**
 * @brief Turn on the LED by setting the GPIO pin to HIGH.
 * @param pin GPIO pin number associated with the LED.
 * @details
 * This method uses the CMSIS-Driver GPIO interface to set the pin state.
 */
void Led::on(uint32_t pin) {
  // Set the pin to HIGH (logic 1) to turn on the LED
  Driver_GPIO0.SetOutput(pin, 1U);
}

/**
 * @brief Turn off the LED by setting the GPIO pin to LOW.
 * @param pin GPIO pin number associated with the LED.
 * @details
 * This method uses the CMSIS-Driver GPIO interface to set the pin state.
 */
void Led::off(uint32_t pin) {
  // Set the pin to LOW (logic 0) to turn off the LED
  Driver_GPIO0.SetOutput(pin, 0U);
}

/**
 * @brief Toggle the state of the LED.
 * @param pin GPIO pin number associated with the LED.
 * @details
 * This method reads the current state of the GPIO pin associated with the LED.
 * If the pin is LOW (logic 0), it sets it HIGH (logic 1), and vice versa.
 * The implementation uses the CMSIS-Driver `ARM_DRIVER_GPIO` interface.
 */
void Led::toggle(uint32_t pin) {
  // Read the current state of the pin
  uint32_t pin_state = Driver_GPIO0.GetInput(pin);

  // Apply the new output state to the pin
  Driver_GPIO0.SetOutput(pin, pin_state == 0 ? 1U : 0U);
  // Note: The pin state is inverted here to toggle the LED
}
