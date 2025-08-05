#include "led.h"
#include "Driver_GPIO.h"

void Led::toggle(void) {
	
    extern ARM_DRIVER_GPIO Driver_GPIO0;
	
    uint32_t pin_state = 0U;
    if (Driver_GPIO0.GetInput(this->pin) == 0) {
        pin_state = 1U;
    }
    Driver_GPIO0.SetOutput(pin, pin_state);
}