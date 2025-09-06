/**
 * @file    fs_log.h
 * @brief   File system logging interface for embedded application.
 * @details
 *   This header declares the FsLog singleton class, which provides methods for
 *   logging messages to the file system, replaying logs to USB, and managing
 *   thread-safe access using RTOS primitives.
 */

#ifndef FS_LOG_H
#define FS_LOG_H

#include "stdint.h"
#include <cstdint>
#ifdef __cplusplus

/**
 * @class   FsLog
 * @brief   Singleton class for file system logging.
 * @details
 * Provides methods for initializing the file system logger, logging messages,
 * and replaying logs to USB. Thread safety is ensured using RTOS primitives.
 */
class FsLog {
public:
  static FsLog &getInstance();

  void init();

  void log(const char *msg);

  void replayLogsToUsb();

  void fsLogThread();

private:
  FsLog();
  FsLog(const FsLog &) = delete;
  FsLog &operator=(const FsLog &) = delete;
  ~FsLog() = default;

  int32_t fsInit; /*!< Initialization status of the file system logger */
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif // FS_LOG_H