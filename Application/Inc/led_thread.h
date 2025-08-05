// LedThread.h
#ifndef __LEDTHREAD_H
#define __LEDTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
 /* USER CODE END Includes */

#ifdef __cplusplus
}
#endif

class LedThread {
public:
    LedThread(const char* name, uint32_t pin);
    void start(void *argument);
    
private:
    static void thread_entry(void* argument);
    void run();

    const char* thread_name;
    uint32_t pin;
    osThreadId_t thread_id = NULL;
		osSemaphoreId_t *sem;

    uint64_t stack[64] __attribute__((aligned(64)));
    uint64_t cb[32] __attribute__((aligned(64)));

    osThreadAttr_t thread_attr;
};

#endif