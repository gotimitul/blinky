// LedThread.cpp
#include "led_thread.h"
#include "led.h"


LedThread::LedThread(const char* name, uint32_t pin)
    : thread_name(name), pin(pin) {

    thread_attr = {
        .name = thread_name,
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
		sem = (osSemaphoreId_t*)argument;
}

void LedThread::thread_entry(void* argument) {
    static_cast<LedThread*>(argument)->run();
}

void LedThread::run() {
    Led led;
    for (;;) {
        osSemaphoreAcquire(sem, osWaitForever);
        led.toggle(pin);
        osDelay(100);
        led.toggle(pin);
        osSemaphoreRelease(sem);
        osDelay(1);
    }
}