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

  enum FsInitStatus {
    FS_NOT_INITIALIZED = 1,      /*!< Not initialized */
    FS_INITIALIZED = 0,          /*!< Initialized */
    FS_FILE_FORMAT_ERROR = -1,   /*!< File format error */
    FS_DRIVE_INIT_ERROR = -2,    /*!< Drive initialization error */
    FS_FORMAT_ERROR = -3,        /*!< Format error */
    FS_MOUNT_ERROR = -4,         /*!< Mount error */
    FS_FILE_CREATE_ERROR = -5,   /*!< File creation error */
    FS_MUTEX_ERROR = -6,         /*!< Mutex creation error */
    FS_MEMPOOL_ERROR = -7,       /*!< Memory pool creation error */
    FS_MEMPOOL_ALLOC_ERROR = -8, /*!< Memory pool allocation error */
  };

  FsInitStatus fsInit = FS_NOT_INITIALIZED; /*!< Initialization status of the
                                          file system logger */
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif    // FS_LOG_H
/** @} */ // end of logger