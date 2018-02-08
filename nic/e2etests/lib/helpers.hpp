#ifndef LIB_HELPER_H
#define LIB_HELPER_H

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Log with timestamp.
 */
static inline void
TLOG(const char *fmt, ...) {
  char   timestr[24];
  struct timeval tv;
  va_list args;

  gettimeofday(&tv, NULL);
  strftime(timestr, 24, "%H:%M:%S", gmtime(&tv.tv_sec));
  printf("[%s.%03ld] ", timestr, tv.tv_usec / 1000);

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

#endif
