

#ifndef FS_LOG_H
#define FS_LOG_H

#include "stdint.h"
#include <cstdint>
#ifdef __cplusplus

/**
 */
class FsLog {
public:
  static FsLog &getInstance();

  std::int32_t init();

  void log(const char *msg);

  void log(const char *msg, uint32_t val);

  void log(const char *msg, const char *str);

  void log(const char *msg, const char *str, uint32_t val);

  void log(const char *msg, const char *str, const char *str2, uint32_t val);

  void fsLogThread();

private:
  FsLog();
  FsLog(const FsLog &) = delete;
  FsLog &operator=(const FsLog &) = delete;
  ~FsLog() = default;

  //  static void FsLogWrapper(void *argument);
};

extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif // FS_LOG_H