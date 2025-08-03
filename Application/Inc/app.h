
#ifndef __APP_H
#define __APP_H


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"

#endif /* __cplusplus */

typedef struct 
{
    uint32_t delay;
    uint32_t pin;
}argument_t;

void app_main(void *argument);

#ifdef __cplusplus
}
#endif /* __APP_H */