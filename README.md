# LED Blinker with CMSIS-RTOS2

This project demonstrates a multi-threaded LED control system for STM32 microcontrollers using CMSIS-RTOS2 and object-oriented C++. Each LED is controlled by its own thread, synchronized using a counting semaphore to ensure exclusive GPIO access.

---

## ?? Features

- Multi-threaded LED control using CMSIS-RTOS2
- Semaphore-based GPIO multiplexing
- Object-oriented abstraction for LEDs
- Static memory allocation (RTOS-compliant)
- CMSIS-Driver based GPIO access

---

## ?? Project Structure

