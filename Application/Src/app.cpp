#include "app.h"
#include "led.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

osSemaphoreId_t semMultiplex;

	
void led_toggle (void *argument) {
	
	const argument_t *arg = (argument_t*)argument;
	uint32_t pin = arg->pin;
  Led led;

    // ...
	for(;;)	{
		osSemaphoreAcquire(semMultiplex, osWaitForever);
		
		osStatus_t status;                    // capture the return status
		uint32_t   delayTime;                 // delay time in milliseconds

		led.toggle(arg->pin);
				
		delayTime = 100U;                    // delay 1 second
		status = osDelay(delayTime);          // suspend thread execution

		led.toggle(arg->pin);

		osSemaphoreRelease(semMultiplex);

		osDelay(1);
	}
}

void app_main(void *argument) {

	osThreadId_t tid1;
	osThreadId_t tid2;
	osThreadId_t tid3;
	osThreadId_t tid4;
	
	static uint64_t mem_stack[4][32] __attribute__((aligned(64)));
	static uint64_t mem_cb[4][16] __attribute__((aligned(64)));

	static const argument_t arg1 = {.delay = 0, .pin = LED_BLUE_PIN};		
	static const argument_t arg2 = {.delay = 0, .pin = LED_RED_PIN};		
	static const argument_t arg3 = {.delay = 0, .pin = LED_ORANGE_PIN};	
	static const argument_t arg4 = {.delay = 0, .pin = LED_GREEN_PIN};

	const osThreadAttr_t led_blue_thread_config = {.name = "blue", .cb_mem = mem_cb[0], .cb_size = sizeof(mem_cb[0]), .stack_mem = mem_stack[0], .stack_size = sizeof(mem_stack[0]), .priority = osPriorityNormal};	
	const osThreadAttr_t led_red_thread_config = {.name = "red", .cb_mem = mem_cb[1], .cb_size = sizeof(mem_cb[1]), .stack_mem = mem_stack[1], .stack_size = sizeof(mem_stack[1]), .priority = osPriorityNormal};	
	const osThreadAttr_t led_orange_thread_config = {.name = "orange", .cb_mem = mem_cb[2], .cb_size = sizeof(mem_cb[2]), .stack_mem = mem_stack[2], .stack_size = sizeof(mem_stack[2]), .priority = osPriorityNormal};	
	const osThreadAttr_t led_green_thread_config = {.name = "green", .cb_mem = mem_cb[3], .cb_size = sizeof(mem_cb[3]), .stack_mem = mem_stack[3], .stack_size = sizeof(mem_stack[3]), .priority = osPriorityNormal};	

	semMultiplex = osSemaphoreNew(4, 1, NULL);

	tid1 = osThreadNew(led_toggle, (void*)&arg1, &led_blue_thread_config);
	tid2 = osThreadNew(led_toggle, (void*)&arg2, &led_red_thread_config);
	tid3 = osThreadNew(led_toggle, (void*)&arg3, &led_orange_thread_config);
	tid4 = osThreadNew(led_toggle, (void*)&arg4, &led_green_thread_config);
	
	osThreadExit();
}

#ifdef __cplusplus
}
#endif
