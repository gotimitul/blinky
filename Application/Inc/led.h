#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
 /* USER CODE END Includes */

#ifdef __cplusplus
} 
#endif

#define LED_BLUE_PIN        63U
#define LED_RED_PIN         62U
#define LED_ORANGE_PIN      61U
#define LED_GREEN_PIN       60U

class Led {
    private:
		public:
    void toggle(uint32_t pin);
};

#endif /* __LED_H */