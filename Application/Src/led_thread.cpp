// LedThread.cpp
#include "led_thread.h"
#include "led.h"


LedThread::LedThread(const char* name, uint32_t pin)
	: Led(pin)  {

    thread_attr = {
        .name = name,
				.attr_bits = 0U,
        .cb_mem = cb,
        .cb_size = sizeof(cb),
        .stack_mem = stack,
        .stack_size = sizeof(stack),
        .priority = osPriorityNormal
    };
}

void LedThread::start(void *argument) {
    osThreadNew(thread_entry, this, &thread_attr);
		this->sem = (osSemaphoreId_t*)argument;
}

void LedThread::thread_entry(void* argument) {
    LedThread *thread = static_cast<LedThread*>(argument);
	thread->run();
}

void LedThread::run() {
		Led *led = static_cast<Led*>(this);
    for (;;) {
        osSemaphoreAcquire(this->sem, osWaitForever);

        led->toggle();
        osDelay(100);

        led->toggle();
        osSemaphoreRelease(this->sem);
        osDelay(1);
    }
}