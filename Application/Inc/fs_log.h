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

#include <array>
#include <cmsis_os2.h>
#include <cstdint>
#include <logger.h>
#include <string_view>

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
  /** @brief Status codes for file system logger initialization */
  enum FsLogStatus : std::int8_t {
    FS_NOT_INITIALIZED = 2,      /*!< Not initialized */
    FS_TO_USB_OK = 1,            /*!< Successfully replayed logs to USB */
    FS_INITIALIZED = 0,          /*!< Initialized */
    FS_FILE_FORMAT_ERROR = -1,   /*!< File format error */
    FS_DRIVE_INIT_ERROR = -2,    /*!< Drive initialization error */
    FS_FORMAT_ERROR = -3,        /*!< Format error */
    FS_MOUNT_ERROR = -4,         /*!< Mount error */
    FS_FILE_CREATE_ERROR = -5,   /*!< File creation error */
    FS_MUTEX_ERROR = -6,         /*!< Mutex creation error */
    FS_MEMPOOL_ERROR = -7,       /*!< Memory pool creation error */
    FS_MEMPOOL_ALLOC_ERROR = -8, /*!< Memory pool allocation error */
    FS_TO_USB_INIT_ERROR = -9,   /*!< Error replaying logs to USB */
    FS_TO_USB_FILE_OPEN_ERROR =
        -10, /*!< Error opening log file for USB replay */
  };

  static FsLog &getInstance(); /*!< Get singleton instance */

  void init(); /*!< Initialize logger */

  void log(std::string_view msg) override; /*!< Log a message */

  FsLog::FsLogStatus replayLogsToUsb(); /*!< Replay logs to USB */

private:
  FsLog();                                  /*!< Singleton */
  FsLog(const FsLog &) = delete;            /*!< Prevent copy construction */
  FsLog &operator=(const FsLog &) = delete; /*!< Prevent assignment */
  ~FsLog() = default;                       /*!< Default destructor */

  osMemoryPoolId_t fsMemPoolId;    /*!< Memory pool ID for log buffer */
  osMutexId_t fsMutexId;           /*!< Mutex ID for file system access */
  osThreadId_t threadId = nullptr; /*!< RTOS thread ID for logger */

  std::array<char, 16> file_path;      /*!< Full path for log file */
  std::atomic_uint32_t cursor_pos = 0; /*!< Cursor position for reading logs */

  int32_t fs_recreate(int32_t &fd);
  void logsToFs(std::string_view msg);
  FsLog::FsLogStatus fsLogsToUsb(); /*!< Logger thread function */

  FsLogStatus fsInit = FsLogStatus::FS_NOT_INITIALIZED; /*!< Initialization
                                          status of the file system logger */
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif    // FS_LOG_H
/** @} */ // end of logger