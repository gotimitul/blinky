/**
 * @file logger.h
 * @brief Logger interface for different logging mechanisms.
 * @version 1.0
 * @date 2025-08-07
 * @author Mitul Goti
 * @defgroup Logger Logger Interface
 * @{
 * @details Logger interface for different logging mechanisms.
 *   This header defines the Logger abstract base class which can be extended
 *   by various logging implementations such as USB logging and file system
 *   logging. It provides a common interface for logging messages across
 *   different mechanisms.
 */

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus

/**
 * @class   Logger
 * @brief   Abstract base class for logging.
 * @details This class defines the interface for logging messages.
 */
class Logger {
public:
  virtual ~Logger() = default;           /*!< Virtual destructor */
  virtual void log(const char *msg) = 0; /*!< Pure virtual log method */
};
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif    // LOGGER_H
/** @} */ // end of logger