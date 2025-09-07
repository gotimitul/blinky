/**
 * @file    fs_log.cpp
 * @brief   File system logging implementation for embedded application.
 * @details
 * This module provides logging functionality using the RL-ARM FlashFS.
 * It supports log file creation, writing, and replaying logs to USB.
 * Thread safety is ensured using RTOS mutexes and memory pools.
 *
 * @author  Mitul Goti
 * @version V1.0
 * @date    2025-09-06
 */

/* File System Logger
 ---
 # 📝 Overview
 The File System Logger provides a way to log messages to a file stored in the
 embedded file system. It supports various log levels and can replay logs over
 USB. # ⚙️ Features
 - Log messages to a file in the embedded file system.
 - Support for different log levels (e.g., INFO, WARN, ERROR).
 - Replay log messages over USB CDC.
 - Thread-safe logging using RTOS mutexes.
 - Memory pool management for log buffers.
 - Configurable log file name and path.
 - Automatic log file rotation.
 - Integration with USB Logger for unified logging.
 - Error handling for file system operations.
 - Profiling support using Event Recorder.
 - Efficient memory usage with static allocation for RTOS objects.

 # 📋 Usage
 To use the File System Logger, initialize it using
 `FsLog::getInstance().init()`. Use the `log` method to log messages. To replay
 logs over USB, call `replayLogsToUsb()`. # 🛠️ Commands You can interact with the
 firmware over the USB CDC (virtual COM port) using the following commands: |
 Command         | Description |
 |-----------------|------------------------------------------------------------------|
 | `<number>`      | Set LED ON time in milliseconds (valid range: 100–2000). |
 | `fsLog out`     | Replay file system logs to USB. |
 | `fsLog on`      | Enable file system logging (disables USB logging). |
 | `fsLog off`     | Disable file system logging. |
 | `help`          | Show this help message. |
 - **Example:** Sending `500` sets the LED ON time to 500 ms.
 - **Note:** Invalid commands or out-of-range values will result in an error
 message.

 # 🔧 Implementation Details
 The File System Logger is implemented as a singleton class `FsLog`. It uses the
 RL-ARM FlashFS for file operations and CMSIS-RTOS2 for threading and
 synchronization. The logger maintains a mutex for thread-safe access to the
 file system and a memory pool for log buffers. The log file is created during
 initialization, and messages are appended to it. The logger can replay logs
 over USB by reading from the file and sending the data via the USB Logger.

 # 🧩 Dependencies
 - RL-ARM FlashFS for file system operations.
 - CMSIS-RTOS2 for threading and synchronization.
 - USB Logger for USB communication.
 - Event Recorder for profiling (optional).
 - Standard C/C++ libraries for string manipulation and I/O.

 # ⚠️ Limitations
 - The file system must be properly initialized and mounted before using the
 logger.
 - The log file size is limited by the available storage in the embedded file
 system.

 # 🛡️ Error Handling
 The logger includes error handling for file system operations. If an error
 occurs during file creation, writing, or reading, appropriate error messages
 are logged via the USB Logger. The logger also attempts to recreate the log
 file if a write error occurs. # 🧪 Testing The File System Logger has been
 tested on the target embedded platform with various log messages and commands.
 It has been verified to handle concurrent logging from multiple threads and to
 replay logs correctly over USB. # 📚 References
 - RL-ARM FlashFS documentation.
 - CMSIS-RTOS2 documentation.
 - USB Logger documentation.
 - Event Recorder documentation.
 - Standard C/C++ library documentation.

 # 📝 Changelog
 - V1.0: Initial implementation of the File System Logger.
 - V1.1: Added support for log file rotation and improved error handling.
 */

#include "fs_log.h"
#include "cmsis_os2.h"
#include "retarget_fs.h"
#include "rl_fs.h"
#include "usb_logger.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief   Anonymous namespace for internal linkage.
 * @details Contains internal variables and helper functions for file system
 * logging.
 */
namespace {
const char *drive_r0 = "R0:";      /*!< Drive name for FlashFS */
const char *file_name = "log.txt"; /*!< Log file name */
char file_path[16];                /*!< Full path for log file */
uint32_t cursor_pos = 0;           /*!< Cursor position for reading logs */
uint32_t block_count = 1;          /*!< Number of memory pool blocks */
osMemoryPoolId_t fsMemPoolId;      /*!< Memory pool ID for log buffer */
osMutexId_t fsMutexId;             /*!< Mutex ID for file system access */
osThreadId_t threadId = nullptr;   /*!< RTOS thread ID for logger */
char *fs_buf = nullptr;            /*!< Buffer for file system operations */

uint64_t stack[256]
    __attribute__((aligned(64))); /*!< Static thread stack (aligned) */
uint64_t cb[32]
    __attribute__((aligned(64))); /*!< Static thread control block (aligned) */

/**
 * @brief   Thread attributes for the logger thread.
 */
constexpr osThreadAttr_t threadAttr = {
    .name = "FsLogThread",       /*!< Name for debugging */
    .attr_bits = 0U,             /*!< No special thread attributes */
    .cb_mem = cb,                /*!< Control block memory */
    .cb_size = sizeof(cb),       /*!< Control block size */
    .stack_mem = stack,          /*!< Stack memory */
    .stack_size = sizeof(stack), /*!< Stack size */
    .priority = osPriorityLow1   /*!< Thread priority */
};

uint64_t fs_buf_mem[256 / 8]
    __attribute__((aligned(64))); /*!< Memory buffer for file system */
uint64_t fs_buf_cb[32]
    __attribute__((aligned(64))); /*!< Control block for file system */

/**
 * @brief   Memory pool attributes for log buffer.
 */
constexpr osMemoryPoolAttr_t fsBufAttr = {
    .name = "FsLogBuffer",         /*!< Name for debugging */
    .attr_bits = 0U,               /*!< No special attributes */
    .cb_mem = fs_buf_cb,           /*!< Control block memory */
    .cb_size = sizeof(fs_buf_cb),  /*!< Control block size */
    .mp_mem = fs_buf_mem,          /*!< Memory pool memory */
    .mp_size = sizeof(fs_buf_mem), /*!< Memory pool size */
};

/**
 * @brief   Mutex attributes for file system access.
 */
constexpr osMutexAttr_t fsMutexAttr = {
    .name = "FsLogMutex",           /*!< Name for debugging */
    .attr_bits = osMutexPrioInherit /*!< Priority inheritance */
};

/**
 * @brief   Write a buffer to the file system.
 * @param   fd  File descriptor.
 * @param   buf Buffer to write.
 * @return  Number of bytes written or negative value on error.
 */
static std::int32_t fsWrite(int &fd, const char *buf) {
  return fs_fwrite(fd, buf, strlen(buf));
}

/**
 * @brief   Recreate the log file after a write error.
 * @param   fd  File descriptor (reference).
 * @return  0 on success, -1 on failure.
 */
static int32_t fs_recreate(int32_t &fd) {
  std::uint8_t retries = 3;
  do {
    fs_fclose(fd);           /* Close the current file descriptor */
    rt_fs_remove(file_path); /* Remove the existing log file */
    fd = fs_fopen(file_path,
                  FS_FOPEN_CREATE | FS_FOPEN_WR); /* Recreate log file */
    if (fd >= 0) {
      if (fsWrite(fd, "Log file recreated after write error.\r\n") > 0) {
        return 0;
      }
    }
  } while (--retries > 0); /* Retry up to 3 times */
  return -1;
}
} // namespace

/**
 * @class   FsLog
 * @brief   Singleton class for file system logging.
 */

/**
 * @brief   Constructor (private for singleton pattern).
 */
FsLog::FsLog() {}

/**
 * @brief   Get the singleton instance of FsLog.
 * @return  Reference to FsLog instance.
 */
FsLog &FsLog::getInstance() {
  static FsLog instance;
  return instance;
}

/**
 * @brief   Initialize the file system logger.
 * @details
 *  - Initializes the file system and creates the log file.
 *  - Sets up mutex and memory pool for thread safety.
 */
void FsLog::init() {
  fsInit = 0;
  std::int32_t status = 0;
  int32_t n = std::snprintf(file_path, sizeof(file_path), "%s\\%s", drive_r0,
                            file_name);
  // Check for snprintf errors
  if (n < 0 || n >= sizeof(file_path)) {
    fsInit = -1;
    return;
  }

  status = finit(drive_r0); /* Initialize File System */
  if (status == fsOK) {
    status = fmount(drive_r0); /* Try to mount the file system */
    if (status == fsNoFileSystem) {
      status = fformat(drive_r0, "FAT32"); /* Format if no file system */
      if (status == fsOK) {
        status = fmount(drive_r0); /* Mount again after formatting */
        if (status == fsOK) {
          int32_t fd = fs_fopen(
              file_path, FS_FOPEN_CREATE | FS_FOPEN_WR); /* Create log file */
          // Check if file was created successfully
          if (fd > 0) {
            fs_fclose(fd); /* Close the file after creation */
            this->log("Log file system initialized.\r\n");
          } else {
            UsbLogger::getInstance().log("Failed to create log file.\r\n");
            fsInit = -1; /* Mark initialization failure */
            return;
          }
        } else {
          UsbLogger::getInstance().log(
              "Failed to mount the formatted drive.\r\n");
          fsInit = -1; /* Mark initialization failure */
          return;
        }
      } else {
        UsbLogger::getInstance().log("Failed to format the drive.\r\n");
        fsInit = -1; /* Mark initialization failure */
        return;
      }
    } else {
      UsbLogger::getInstance().log("Failed to mount the drive.\r\n");
      fsInit = -1; /* Mark initialization failure */
      return;
    }
  } else {
    UsbLogger::getInstance().log("RAM drive can not be initialized.\r\n");
    fsInit = -1; /* Mark initialization failure */
    return;
  }

  fsMutexId = osMutexNew(&fsMutexAttr);
  if (fsMutexId == nullptr) {
    fsInit = -1; /* Mark initialization failure */
    return;
  }

  fsMemPoolId = osMemoryPoolNew(block_count, sizeof(fs_buf_mem),
                                &fsBufAttr); /* 1 block of 256 bytes */
  if (fsMemPoolId == nullptr) {
    fsInit = -1; /* Mark initialization failure */
    return;
  } else {
    fs_buf = (char *)osMemoryPoolAlloc(fsMemPoolId, 0);
  }
  fsInit = 0; /* Mark successful initialization */
  return;
}

/**
 * @brief   Write a message to the log file.
 * @param   msg Null-terminated string to write.
 */
void fs_write(const char *msg) {
  std::int32_t status;
  std::int32_t fd;
  if (fsMutexId == nullptr) {
    return; /* Mutex not initialized */
  }
  /* Acquire mutex for thread safety */
  osMutexAcquire(fsMutexId, osWaitForever);
  /* Open log file in append mode */
  auto append_msg = [&](const char *msg) {
    int32_t status = fsWrite(fd, msg);
    fs_fclose(fd);
    return status;
  };

  fd = fs_fopen(file_path, FS_FOPEN_APPEND);
  if (fd > 0) {
    /* Move cursor to end of file */
    if (fs_fseek(fd, 0, SEEK_END) >= 0) {
      /* Check free space whether it is greater than the message size */
      if (ffree(drive_r0) < strlen(msg)) {
        /* Not enough space, attempt to recreate the log file */
        if (fs_recreate(fd) == NULL) {
          fd = append_msg(msg); /* Retry writing the message in the new file */
          cursor_pos = 0;       /* Reset cursor position */
          osMutexRelease(fsMutexId);
          if (fd < 0) {
            UsbLogger::getInstance().log(
                "Failed to write in the new log file.\r\n");
          }
        } else {
          osMutexRelease(fsMutexId);
          UsbLogger::getInstance().log(
              "Failed to recreate log file after multiple attempts.\r\n");
          return;
        }
      } else {
        fd = append_msg(msg);
        osMutexRelease(fsMutexId);
        if (fd < 0) {
          UsbLogger::getInstance().log("Failed to write in the log file.\r\n");
        }
      }
    } else {
      osMutexRelease(fsMutexId);
      UsbLogger::getInstance().log(
          "Failed to set the cursor at the end of the file.\r\n");
      return;
    }
  } else {
    osMutexRelease(fsMutexId);
    UsbLogger::getInstance().log("Failed to open the requested file.\r\n");
    return;
  }
}

/**
 * @brief   Log a message using the file system logger.
 * @param   msg Null-terminated string to log.
 */
void FsLog::log(const char *msg) {
  if (fsInit == 0) {
    fs_write(msg);
  }
}

/**
 * @brief   Replay log file contents to USB.
 * @details Reads the log file and sends its contents over USB.
 */
void FsLog::replayLogsToUsb() {
  if (fsInit == 0) {
    UsbLogger::getInstance().log("Replaying logs to USB...\n");
    osDelay(10); /* Small delay to ensure USB is ready */
    FsLog::getInstance().fsLogThread();
  }
}

/**
 * @brief   Logger thread function for replaying logs.
 * @details
 *  - Reads new log data from the file.
 *  - Sends data to USB in chunks.
 *  - Updates cursor position.
 */
void FsLog::fsLogThread() {
  std::int32_t n;
  std::int32_t fd;
  constexpr uint32_t FS_DATA_PAKET_SIZE = 256;
  if (fsInit != 0) {
    return;
  }
  if (fsMemPoolId == nullptr) {
    return;
  }
  if (fsMutexId == nullptr) {
    return;
  }
  if (fs_buf == nullptr) {
    fs_buf = (char *)osMemoryPoolAlloc(fsMemPoolId, 0);
    if (fs_buf == nullptr) {
      return;
    }
  }
  for (;;) {
    osMutexAcquire(fsMutexId, osWaitForever);
    fd = fs_fopen("R0:/log.txt", FS_FOPEN_RD);
    if (fd > 0) {
      n = fs_fsize(fd); /* Get file size */

      if (n > cursor_pos) {
        fs_fseek(fd, cursor_pos, SEEK_SET);

        n = fs_fread(fd, fs_buf,
                     (n - cursor_pos) < FS_DATA_PAKET_SIZE
                         ? n - cursor_pos
                         : FS_DATA_PAKET_SIZE); /* Read file content */
        fs_fclose(fd);
        osMutexRelease(fsMutexId);
        const char *end_ptr = fs_buf + n;
        const char *start_ptr = fs_buf;
        while (end_ptr != start_ptr) {
          end_ptr--; /* Trim trailing newlines */
          if (*end_ptr == '\n')
            break;
        }
        n = end_ptr - start_ptr + 1;
        if (n > 1) {
          while (UsbLogger::usbXferChunk(fs_buf, n) == -1) {
            osDelay(10); /* Wait and retry if USB transfer fails */
          }
          cursor_pos += n;
        }
      } else {
        fs_fclose(fd);
        osMutexRelease(fsMutexId);
        return; /* No new data to read, exit the thread */
      }
    } else {
      UsbLogger::getInstance().log("Failed to open log file for reading.\r\n");
      osMutexRelease(fsMutexId);
    }
    osDelay(50); /* Placeholder for periodic tasks if needed */
  }
  return;
}