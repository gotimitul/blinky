# LED Blinker with CMSIS-RTOS2

This project implements a modular, multi-threaded LED control and logging system for STM32 microcontrollers, specifically targeting the STM32F4 Discovery board. It uses CMSIS-RTOS2 for real-time threading, RL-ARM FlashFS for file system logging, and USB CDC for host communication. The codebase is fully documented with Doxygen and is designed for extensibility and reliability.

---

## ✨ Features

- **Multi-threaded LED Control:** Each LED is managed by its own thread (`LedThread`), with thread-safe GPIO access using a counting semaphore.
- **Event-Driven Architecture:** Button events and inter-thread communication via event flags and semaphores.
- **Object-Oriented Design:** Modular classes for LED control, logging, and routing (`Led`, `LedThread`, `FsLog`, `UsbLogger`, `LogRouter`).
- **Static Memory Allocation:** All RTOS objects (threads, mutexes, semaphores, memory pools) use static allocation for reliability.
- **File System Logging:** Log messages to a file on the RL-ARM FlashFS RAM disk (`FsLog`).
- **USB CDC Logging:** Real-time log output and command interface over USB (`UsbLogger`).
- **Log Replay:** Replay log file contents to USB on button press or command.
- **Logging Router:** Unified interface to route logs to file system, USB, or both (`LogRouter`).
- **Command Interface:** Change LED blink timing and trigger log replay via USB commands.
- **Debug Support:** EventRecorder and printf-based debug output.
- **Extensible:** Easily add more LEDs, logging backends, or features.
- **Doxygen Documentation:** All code is documented for easy reference and maintainability.
- **Application Layer:** Centralized application logic in `app.cpp`/`app.h`.
- **Boot Clock:** Timekeeping and timestamping support via `boot_clock.cpp`/`boot_clock.h`.

---

## 📁 Project Structure

- `Application/Inc/` – Header files:
  - `app.h` – Main application entry and configuration
  - `boot_clock.h` – Boot time and timestamp utilities
  - `fs_log.h` – File system logger
  - `led_thread.h` – LED thread management
  - `led.h` – LED control abstraction
  - `log_router.h` – Logging router
  - `logger.h` – Virtual base class for logging APIs
  - `usb_logger.h` – USB CDC logger
- `Application/Src/` – Source files:
  - `app.cpp` – Main application logic and initialization
  - `boot_clock.cpp` – Boot time and timestamp implementation
  - `fs_log.cpp` – File system logging implementation
  - `led_thread.cpp` – LED thread logic
  - `led.cpp` – LED control implementation
  - `log_router.cpp` – Logging router implementation
  - `usb_logger.cpp` – USB CDC logging implementation
- `.vscode/` – VS Code configuration (launch, tasks)
- `out/` – Build output directory

---

## 🚦 How It Works

- **LED Threads:** Each `LedThread` instance controls a specific LED, toggling it at a configurable interval. Threads synchronize GPIO access using a shared semaphore.
- **Button Events:** Pressing the blue user button sets an event flag, which can trigger log replay or other actions.
- **Logging:** Use `FsLog` for file system logging and `UsbLogger` for USB CDC logging. The `LogRouter` class allows unified logging to both backends.
- **Log Replay:** Press the button or send a USB command to replay the log file contents over USB.
- **USB Command Interface:** Send commands over USB CDC to change LED blink timing or request log replay.
- **Thread Safety:** All logging and hardware access is protected by mutexes, semaphores, and memory pools.
- **Boot Clock:** Provides timestamps for log entries and event tracking.
- **Application Layer:** The `app.cpp`/`app.h` files coordinate initialization and high-level application flow.

---

## 🛠️ Requirements

- **Hardware:** STM32F4 Discovery board (tested platform)
- **Debugger:** ST-Link or J-Link (for flashing and debugging)
- **Software:**
  - CMSIS-RTOS2 (RTX5 or FreeRTOS)
  - RL-ARM FlashFS (for file system logging)
  - USB CDC middleware (for USB logging)
  - Arm toolchain (`arm-none-eabi-gcc`)
  - VS Code with Cortex-Debug extension (optional)
  - Doxygen (for documentation generation)

---

## 🚀 Getting Started

1. **Clone this repository.**
2. **Open the project in VS Code.**
3. **Configure your debugger and build tasks** in `.vscode/launch.json` and `.vscode/tasks.json`.
4. **Build and flash the firmware** to your STM32F4 Discovery board.
5. **Connect via USB** to view logs or send commands (e.g., change LED timing or trigger log replay).
6. **Press the blue user button** to replay logs from the file system to USB.

---

## 📝 Documentation

- All source and header files are documented using Doxygen-style comments.
- To generate HTML documentation, run `doxygen` in the project root (requires Doxygen installed).

---

##

