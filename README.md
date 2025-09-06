# LED Blinker with CMSIS-RTOS2

This project demonstrates a multi-threaded LED control system for STM32 microcontrollers using CMSIS-RTOS2 and object-oriented C++. Each LED is controlled by its own thread, synchronized using a counting semaphore to ensure exclusive GPIO access.

---

## ✨ Features

- Multi-threaded LED control using CMSIS-RTOS2
- Semaphore-based GPIO multiplexing
- Object-oriented abstraction for LEDs
- Static memory allocation (RTOS-compliant)
- CMSIS-Driver based GPIO access
- File system logging with RL-ARM FlashFS
- USB CDC logging and command interface
- Thread-safe log replay to USB

---

## 📁 Project Structure

- `Application/Inc/` – Header files (e.g., `led_thread.h`, `fs_log.h`, `usb_logger.h`)
- `Application/Src/` – Source files (e.g., `led_thread.cpp`, `fs_log.cpp`, `usb_logger.cpp`)
- `.vscode/` – VS Code configuration (launch, tasks)
- `out/` – Build output directory

---

## 🚦 How It Works

- Each LED is managed by a dedicated thread (`LedThread`), which toggles the LED and responds to button events.
- A counting semaphore ensures only one thread accesses the GPIO at a time.
- Logging is performed to both the file system and USB CDC, with replay and command features.
- Press the blue user button to flush out the log data from the file system.
- All RTOS objects use static memory allocation for reliability.

---

## 🛠️ Requirements

- STM32F4 Discovery board (tested hardware platform)
- ST-Link or J-Link debugger (for flashing and debugging)
- CMSIS-RTOS2 (FreeRTOS)
- RL-ARM FlashFS (for file system logging)
- USB CDC middleware (for USB logging)
- Arm toolchain (`arm-none-eabi-gcc`)
- VS Code with Cortex-Debug extension (optional)

---

## 🚀 Getting Started

1. Clone this repository.
2. Open the project in VS Code.
3. Configure your debugger and build tasks in `.vscode/launch.json` and `.vscode/tasks.json`.
4. Build and flash the firmware to your STM32 board.
5. Connect via USB to view logs or send commands.

---

## 📄 License

