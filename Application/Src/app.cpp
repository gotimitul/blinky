#include "app.h"
#include "led.h"
#include "cmsis_os2.h"
#include "led_thread.h"

#ifdef __cplusplus
extern "C" {
#endif
	
void app_main(void *argument) {

	UNUSED(argument);
	osSemaphoreId_t semMultiplex;
	semMultiplex = osSemaphoreNew(4, 1, NULL);

	static LedThread blue("blue", LED_BLUE_PIN);
	static LedThread red("red", LED_RED_PIN);
	static LedThread orange("orange", LED_ORANGE_PIN);
	static LedThread green("green", LED_GREEN_PIN);
	
	blue.start(semMultiplex);
	red.start(semMultiplex);
	orange.start(semMultiplex);
	green.start(semMultiplex);

	osThreadExit();
}

#ifdef __cplusplus
}
#endif
