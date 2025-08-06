/**
 * @file led.cpp
 * @brief Implementation of the Led class for GPIO-based LED control
 *
 * This file defines the toggle method of the Led class, using the CMSIS-compliant
 * ARM GPIO driver to change the LED state based on its current input.
 */

#include "led.h"
#include "Driver_GPIO.h"  // CMSIS-Driver GPIO interface

/**
 * @brief Toggle the state of the LED
 *
 * This method reads the current state of the GPIO pin associated with the LED.
 * If the pin is LOW (logic 0), it sets it HIGH (logic 1), and vice versa.
 *
 * The implementation uses the CMSIS-Driver `ARM_DRIVER_GPIO` interface.
 */
void Led::toggle(void) {
    // Use external GPIO driver instance
    extern ARM_DRIVER_GPIO Driver_GPIO0;

    uint32_t pin_state = 0U;

    // Read the current state of the pin
    if (Driver_GPIO0.GetInput(this->pin) == 0) {
        // If currently LOW, prepare to set it HIGH
        pin_state = 1U;
    }

    // Apply the new output state to the pin
    Driver_GPIO0.SetOutput(pin, pin_state);
}
