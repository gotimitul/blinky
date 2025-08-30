

#include "fs_log.h"
#include "cmsis_os2.h"
#include "retarget_fs.h"
#include "rl_fs.h"
#include "usb_logger.h"
#include <cstdint>
#include <cstring>
#include <memory>
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

namespace {
uint32_t page_count = 0;
osThreadId_t threadId = nullptr; ///< RTOS thread ID for logger

uint64_t stack[2048]
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
    .priority = osPriorityLow    // Thread priority
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

/**
 */
std::int32_t FsLog::init() {
  // Initialization code if needed
  std::int32_t status = 0;
  status = finit("R0:"); // Initialize File System
  if (status == fsOK) {
    // Try to mount the file system
    status = fmount("R0:");
    if (status == fsNoFileSystem) {
      // If no file system, format the drive
      status = fformat("R0:", "FAT32");
      if (status == fsOK) {
        status = fmount("R0:"); // Try to mount again after formatting
        if (status == fsOK) {
          int32_t fd = fs_fopen("R0:\\log.txt", FS_FOPEN_CREATE | FS_FOPEN_WR);
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
  return status;
}

void FsLog::log(const char *msg) {
  std::int32_t status;
  std::int32_t n;
  std::int32_t fd;

  fd = fs_fopen("R0:\\log.txt", FS_FOPEN_APPEND);
  fs_fseek(fd, 0, SEEK_END);
  if (fd >= 0) {
    n = fs_fwrite(fd, msg, strlen(msg));
    if (n <= 0) {
      // Handle write error if needed
      fs_fclose(fd);
      rt_fs_remove("R0:\\log.txt");
      fd = fs_fopen("R0:\\log.txt", FS_FOPEN_CREATE | FS_FOPEN_WR);
      page_count = 0;
      n = fs_fwrite(fd, msg, strlen(msg));
    }
    fs_fclose(fd);
  } else {
    // Handle file open error if needed
  }
}

void FsLog::log(const char *msg, uint32_t val) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, val);
  log(logMsg);
}

void FsLog::log(const char *msg, const char *str) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, str);
  log(logMsg);
}

void FsLog::log(const char *msg, const char *str, uint32_t val) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, str, val);
  log(logMsg);
}

void FsLog::log(const char *msg, const char *str, const char *str2,
                uint32_t val) {
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), msg, str, str2, val);
  log(logMsg);
}

void FsLog::FsLogWrapper(void *argument) {
  if (argument != nullptr) {
    FsLog *logger = static_cast<FsLog *>(argument);
    logger->fsLogThread();
  }
}

void FsLog::fsLogThread() {
  std::int32_t n;
  std::int32_t fd;
  static char fs_buf[256];

  for (;;) {
    uint32_t cursor_pos = page_count * 256;
    fd = fs_fopen("R0:/log.txt", FS_FOPEN_RD);
    if (fd >= 0) {
      n = fs_fsize(fd); // Get file size

      if (n > cursor_pos + 256) {
        fs_fseek(fd, cursor_pos, SEEK_SET);
        n = fs_fread(fd, &fs_buf, 256); // Read file content
                                        //        if (n == 256) {
        // Successfully read data, process it if needed
        while (UsbLogger::usbXferChunk(fs_buf, n) == -1) {
          osDelay(10); // Wait and retry if USB transfer fails
        }
        std::memset(fs_buf, 0, sizeof(fs_buf)); // Clear buffer after use
        page_count++;
      }
    }
    fs_fclose(fd);
    osDelay(1000); // Placeholder for periodic tasks if needed
  }
}