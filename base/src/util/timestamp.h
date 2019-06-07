#ifndef WOLF_UTIL_UTIMESTAMP_H
#define WOLF_UTIL_UTIMESTAMP_H

#include <stdint.h>
#include <string>
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
#include <time.h>
#else
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#endif

#define TM_NANOSEC 1000000000
#define TM_MICROSEC 1000000

namespace wolf {
namespace util {
namespace timestamp {

inline uint64_t getTimeSec() {
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
  struct timespec ti;
  clock_gettime(CLOCK_MONOTONIC, &ti);
  return (int64_t)1000000000 * ti.tv_sec + ti.tv_nsec;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (int64_t)1000000000 * tv.tv_sec + tv.tv_usec * 1000;
#endif
}

inline uint64_t getTime() {
  uint64_t t;
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
  struct timespec ti;
  clock_gettime(CLOCK_MONOTONIC, &ti);
  t = (uint64_t)ti.tv_sec * 100;
  t += ti.tv_nsec / 10000000;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  t = (uint64_t)tv.tv_sec * 100;
  t += tv.tv_usec / 10000;
#endif
  return t;
}

inline void getSystemTime(uint32_t *sec, uint32_t *cs) {
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
  struct timespec ti;
  clock_gettime(CLOCK_REALTIME, &ti);
  *sec = (uint32_t)ti.tv_sec;
  *cs = (uint32_t)(ti.tv_nsec / 10000000);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *sec = tv.tv_sec;
  *cs = tv.tv_usec / 10000;
#endif
}

inline uint64_t getThreadTime() {
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
  struct timespec ti;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ti);

  return (uint64_t)ti.tv_sec * TM_MICROSEC +
         (uint64_t)ti.tv_nsec / (TM_NANOSEC / TM_MICROSEC);
#else
  struct task_thread_times_info aTaskInfo;
  mach_msg_type_number_t aTaskInfoCount = TASK_THREAD_TIMES_INFO_COUNT;
  if (KERN_SUCCESS != task_info(mach_task_self(), TASK_THREAD_TIMES_INFO,
                                (task_info_t)&aTaskInfo, &aTaskInfoCount)) {
    return 0;
  }

  return (uint64_t)(aTaskInfo.user_time.seconds) +
         (uint64_t)aTaskInfo.user_time.microseconds;
#endif
}

inline std::string getTimeLocal() {
  time_t tSetTime;
  time(&tSetTime);
  struct tm *ptm = ::localtime(&tSetTime);
  char tTmp[256];
  sprintf(tTmp, "%d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900,
          ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
          ptm->tm_sec);
  return std::string(tTmp);
}

} // namespace timestamp
} // namespace util
} // namespace wolf

#endif
