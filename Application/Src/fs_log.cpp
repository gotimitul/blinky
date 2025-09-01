/**
 * @file fs_log.cpp
 *
 */

#include "fs_log.h"
#include "cmsis_os2.h"
#include "retarget_fs.h"
#include "rl_fs.h"
#include "usb_logger.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
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
    FsLog *logger = static_cast<FsLog *>(argument);
    logger->fsLogThread();
  }
};

/**
 */
std::int32_t FsLog::init() {
  // Initialization code if needed
  std::int32_t status = 0;
  std::snprintf(file_path, sizeof(file_path), "%s\\%s", drive_r0, file_name);

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
          fs_fclose(fd);
          log("Log file system initialized.\r\n");
        }
      }
    }
  }
  threadId = osThreadNew(FsLogWrapper, this, &threadAttr);
  if (threadId != nullptr) {
    // Thread created successfully
  } else {
    // Handle thread creation failure if needed
  }

  fsMutexId = osMutexNew(&fsMutexAttr);
  if (fsMutexId == nullptr) { // Handle mutex creation failure if needed
  }

  fsMemPoolId = osMemoryPoolNew(block_count, sizeof(fs_buf_mem),
                                &fsBufAttr); // 1 block of 256 bytes
  return status;
}

auto fs_write = [](const char *msg) {
  std::int32_t status;
  std::int32_t n;
  std::int32_t fd;
  if (fsMutexId != nullptr) {
    osMutexAcquire(fsMutexId, osWaitForever);
  } else {
    return; // Mutex not created
  }
  fd = fs_fopen(file_path, FS_FOPEN_APPEND);
  fs_fseek(fd, 0, SEEK_END);
  if (fd >= 0) {
    n = fs_fwrite(fd, msg, strlen(msg));
    if (n <= 0) {
      // Handle write error if needed
      fs_fclose(fd);
      rt_fs_remove(file_path);
      fd = fs_fopen(file_path, FS_FOPEN_CREATE | FS_FOPEN_WR);
      cursor_pos = 0;
      char init_msg[] = "Log file recreated after write error.\r\n";
      fs_fwrite(fd, init_msg, sizeof(init_msg));
      n = fs_fwrite(fd, msg, strlen(msg));
    }
    fs_fclose(fd);
  } else {
    // Handle file open error if needed
  }
  osMutexRelease(fsMutexId);
};

void FsLog::log(const char *msg) { fs_write(msg); }

void FsLog::log(const char *msg, uint32_t val) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, val);
  fs_write(logMsg);
}

void FsLog::log(const char *msg, const char *str) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, str);
  fs_write(logMsg);
}

void FsLog::log(const char *msg, const char *str, uint32_t val) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, str, val);
  fs_write(logMsg);
}

void FsLog::log(const char *msg, const char *str, const char *str2,
                uint32_t val) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, str, str2, val);
  fs_write(logMsg);
}

void FsLog::fsLogThread() {
  std::int32_t n;
  std::int32_t fd;
  if (fsMemPoolId == nullptr) {
    // Handle memory pool creation failure if needed
    return;
  }
  char *fs_buf = (char *)osMemoryPoolAlloc(fsMemPoolId, 0);
  for (;;) {
    osMutexAcquire(fsMutexId, osWaitForever);
    fd = fs_fopen("R0:/log.txt", FS_FOPEN_RD);
    if (fd >= 0) {
      n = fs_fsize(fd); // Get file size

      if (n > cursor_pos) {
        fs_fseek(fd, cursor_pos, SEEK_SET);

        n = fs_fread(fd, fs_buf,
                     (n - cursor_pos) < 256 ? n - cursor_pos
                                            : 256); // Read file content
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