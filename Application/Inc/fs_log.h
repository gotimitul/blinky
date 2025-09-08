/**
 * @file    fs_log.h
 * @brief   File system logging interface for embedded application.
 * @author  Mitul Goti
 * @version V1.0
 * @date    2025-09-06
 * @ingroup Logger
 * @{
 * @details
 *   This header declares the FsLog singleton class, which provides methods for
 *   logging messages to the file system, replaying logs to USB, and managing
 *   thread-safe access using RTOS primitives.
 */

#ifndef FS_LOG_H
#define FS_LOG_H

#include "stdint.h"
#include <cstdint>
#include <logger.h>
#ifdef __cplusplus

/**
 * @class FsLog
 * @brief Singleton class for file system logging.
 * @details This class provides methods for logging messages to the file system,
 *          replaying logs to USB, and managing thread-safe access using RTOS
 *          primitives.
 */
class FsLog : public Logger {
public:
  static FsLog &getInstance(); /*!< Get singleton instance */

  void init(); /*!< Initialize logger */

  void log(const char *msg) override; /*!< Log a message */

  void replayLogsToUsb(); /*!< Replay logs to USB */

private:
  FsLog();                                  /*!< Singleton */
  FsLog(const FsLog &) = delete;            /*!< Prevent copy construction */
  FsLog &operator=(const FsLog &) = delete; /*!< Prevent assignment */
  ~FsLog() = default;                       /*!< Default destructor */
  void fsLogsToUsb();                       /*!< Logger thread function */

  int32_t fsInit = -1; /*!< Initialization status of the file system logger */
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif    // FS_LOG_H
/** @} */ // end of logger