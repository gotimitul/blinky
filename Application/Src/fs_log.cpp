/**
 * @file fs_log.cpp
 *
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

/*----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *----------------------------------------------------------------------------
 *      Name:    fs_mapi.c
 *      Purpose: File System Management API module
 *      Rev.:    V4.70
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 */

/**
 */
namespace {
const char *drive_r0 = "R0:";
const char *file_name = "log.txt";
char file_path[16];
uint32_t cursor_pos = 0;
uint32_t block_count = 1;
osMemoryPoolId_t fsMemPoolId;
osMutexId_t fsMutexId;
osThreadId_t threadId = nullptr; ///< RTOS thread ID for logger

uint64_t stack[256]
    __attribute__((aligned(64))); ///< Static thread stack (aligned)
uint64_t cb[32]
    __attribute__((aligned(64))); ///< Static thread control block (aligned)

constexpr osThreadAttr_t threadAttr = {
    .name = "FsLogThread",       // Name for debugging
    .attr_bits = 0U,             // No special thread attributes
    .cb_mem = cb,                // No custom control block memory
    .cb_size = sizeof(cb),       // Default size
    .stack_mem = stack,          // No custom stack memory
    .stack_size = sizeof(stack), // Default size
    .priority = osPriorityLow1   // Thread priority
};

uint64_t fs_buf_mem[256 / 8]
    __attribute__((aligned(64))); ///< Memory buffer for file system
uint64_t fs_buf_cb[32]
    __attribute__((aligned(64))); ///< Control block for file system
constexpr osMemoryPoolAttr_t fsBufAttr = {
    .name = "FsLogBuffer",         // Name for debugging
    .attr_bits = 0U,               // No special attributes
    .cb_mem = fs_buf_cb,           // No custom control block memory
    .cb_size = sizeof(fs_buf_cb),  // Default size
    .mp_mem = fs_buf_mem,          // Pointer to memory for pool
    .mp_size = sizeof(fs_buf_mem), // Size of the memory buffer
};
constexpr osMutexAttr_t fsMutexAttr = {
    .name = "FsLogMutex",           // Name for debugging
    .attr_bits = osMutexPrioInherit // No special attributes
};

static std::int32_t fsWrite(int &fd, const char *buf) {
  return fs_fwrite(fd, buf, strlen(buf));
};

static int32_t fs_recreate(int32_t &fd) {
  std::uint8_t retries = 3;
  do {
    // Handle write error if needed
    fs_fclose(fd);
    rt_fs_remove(file_path);
    fd = fs_fopen(file_path, FS_FOPEN_CREATE | FS_FOPEN_WR);
    if (fd >= 0) {
      if (fsWrite(fd, "Log file recreated after write error.\r\n") > 0) {
        return 0;
      }
    }
  } while (--retries > 0);
  return -1;
};
} // namespace

FsLog::FsLog() {}

/** @brief Constructor
 * Private constructor for singleton pattern
 * Ensures only one instance of FsLog exists.
 */
FsLog &FsLog::getInstance() {
  static FsLog instance;
  return instance;
}

auto FsLogWrapper = [](void *argument) {
  if (argument != nullptr) {
    static_cast<FsLog *>(argument)->fsLogThread();
  }
};

/**
 */
void FsLog::init() {
  // Initialization code if needed
  fsInit = 0;
  std::int32_t status = 0;
  int32_t n = std::snprintf(file_path, sizeof(file_path), "%s\\%s", drive_r0,
                            file_name);
  if (n == 0) {
    fsInit = -1;
    return;
  }

  status = finit(drive_r0); // Initialize File System
  if (status == fsOK) {
    // Try to mount the file system
    status = fmount(drive_r0);
    if (status == fsNoFileSystem) {
      // If no file system, format the drive
      status = fformat(drive_r0, "FAT32");
      if (status == fsOK) {
        status = fmount(drive_r0); // Try to mount again after formatting
        if (status == fsOK) {
          int32_t fd = fs_fopen(file_path, FS_FOPEN_CREATE | FS_FOPEN_WR);
          if (fd > 0) {
            fs_fclose(fd);
            log("Log file system initialized.\r\n");
          } else {
            // file with a defined name and path could not be created.
            fsInit = -1;
            return;
          }
        } else {
          // Media driver not initialized.
          fsInit = -1;
          return;
        }
      } else {
        // Undefined drive or formatting failed.
        fsInit = -1;
        return;
      }
    } else {
      // Media driver not initialied.
      fsInit = -1;
      return;
    }
  } else {
    // Drive can not be initialized.
    UsbLogger::getInstance().log("RAM drive can not be initialized.\r\n");
    fsInit = -1;
    return;
  }
  threadId = osThreadNew(FsLogWrapper, this, &threadAttr);
  if (threadId != nullptr) {
    // Thread created successfully
  } else {
    // Handle thread creation failure if needed
    fsInit = -1;
    return;
  }

  fsMutexId = osMutexNew(&fsMutexAttr);
  if (fsMutexId == nullptr) {
    // Handle mutex creation failure if needed
    fsInit = -1;
    return;
  }

  fsMemPoolId = osMemoryPoolNew(block_count, sizeof(fs_buf_mem),
                                &fsBufAttr); // 1 block of 256 bytes
  if (fsMemPoolId == nullptr) {
    // failed to pool a memory block.
    fsInit = -1;
    return;
  }
  fsInit = 0;
  return;
}

void fs_write(const char *msg) {
  std::int32_t status;
  std::int32_t fd;
  if (fsMutexId == nullptr) {
    return;
  }
  osMutexAcquire(fsMutexId, osWaitForever);

  auto append_msg = [&](const char *msg) {
    int32_t status = fsWrite(fd, msg);
    fs_fclose(fd);
    return status;
  };

  fd = fs_fopen(file_path, FS_FOPEN_APPEND);
  if (fd >= 0) {
    if (fs_fseek(fd, 0, SEEK_END) >= 0) {
      if (ffree(drive_r0) < strlen(msg)) {
        if (fs_recreate(fd) == NULL) {
          fd = append_msg(msg);
          cursor_pos = 0;
          osMutexRelease(fsMutexId);
        } else {
          osMutexRelease(fsMutexId);
          UsbLogger::getInstance().log(
              "Failed to recreate log file after multiple attempts.\r\n");
          return;
        }
      } else {
        fd = append_msg(msg);
        osMutexRelease(fsMutexId);
      }
    } else {
      osMutexRelease(fsMutexId);
      UsbLogger::getInstance().log(
          "Failed to set the cursor at the end of the file.\r\n");
      return;
    }
  } else {
    osMutexRelease(fsMutexId);
    // Handle file open error if needed
    UsbLogger::getInstance().log("Failed to open the requested file.\r\n");
    return;
  }
  if (fd < 0) {
    UsbLogger::getInstance().log("Failed to write in the log file.\r\n");
  }
};

void FsLog::log(const char *msg) {
  if (fsInit == 0) {
    fs_write(msg);
  }
}

void FsLog::log(const char *msg, uint32_t val) {
  if (fsInit == 0) {
    char logMsg[64];
    int n = snprintf(logMsg, sizeof(logMsg), msg, val);
    if (n > sizeof(logMsg)) {
      // Warning: Message Size Exceeded. Last Message Truncated.
    }
    fs_write(logMsg);
  }
}

void FsLog::log(const char *msg, const char *str) {
  if (fsInit == 0) {
    char logMsg[64];
    int n = snprintf(logMsg, sizeof(logMsg), msg, str);
    if (n > sizeof(logMsg)) {
      // Warning: Message Size Exceeded. Last Message Truncated.
    }
    fs_write(logMsg);
  }
}

void FsLog::log(const char *msg, const char *str, uint32_t val) {
  if (fsInit == 0) {
    char logMsg[64];
    int n = snprintf(logMsg, sizeof(logMsg), msg, str, val);
    if (n > sizeof(logMsg)) {
      // Warning: Message Size Exceeded. Last Message Truncated.
    }
    fs_write(logMsg);
  }
}

void FsLog::log(const char *msg, const char *str, const char *str2,
                uint32_t val) {
  if (fsInit == 0) {
    char logMsg[64];
    int n = snprintf(logMsg, sizeof(logMsg), msg, str, str2, val);
    if (n > sizeof(logMsg)) {
      // Warning: Message Size Exceeded. Last Message Truncated.
    }
    fs_write(logMsg);
  }
}

void FsLog::fsLogThread() {
  std::int32_t n;
  std::int32_t fd;
  constexpr uint32_t FS_DATA_PAKET_SIZE = 256;
  if (fsInit != 0) {
    return;
  }
  if (fsMemPoolId == nullptr) {
    // Handle memory pool creation failure if needed
    return;
  }
  char *fs_buf = (char *)osMemoryPoolAlloc(fsMemPoolId, 0);
  if (fs_buf == 0) {
    return;
  }
  for (;;) {
    osMutexAcquire(fsMutexId, osWaitForever);
    fd = fs_fopen("R0:/log.txt", FS_FOPEN_RD);
    if (fd >= 0) {
      n = fs_fsize(fd); // Get file size

      if (n > cursor_pos) {
        fs_fseek(fd, cursor_pos, SEEK_SET);

        n = fs_fread(fd, fs_buf,
                     (n - cursor_pos) < FS_DATA_PAKET_SIZE
                         ? n - cursor_pos
                         : FS_DATA_PAKET_SIZE); // Read file content
        fs_fclose(fd);
        osMutexRelease(fsMutexId);
        const char *end_ptr = fs_buf + n;
        const char *start_ptr = fs_buf;
        while (end_ptr != start_ptr) {
          end_ptr--; // Trim trailing newlines
          if (*end_ptr == '\n')
            break;
        }
        n = end_ptr - start_ptr + 1;
        if (n > 0) {
          // Successfully read data, process it if needed
          while (UsbLogger::usbXferChunk(fs_buf, n) == -1) {
            osDelay(10); // Wait and retry if USB transfer fails
          }
          cursor_pos += n;
        }
      } else {
        fs_fclose(fd);
        osMutexRelease(fsMutexId);
      }
    } else {
      osMutexRelease(fsMutexId);
    }
    osDelay(50); // Placeholder for periodic tasks if needed
  }
}