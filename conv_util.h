#ifndef CONV_UTIL_H
#define CONV_UTIL_H

#include <errno.h>
#include <stdlib.h>
#include <limits.h>

typedef enum StrErr {
  NO_ERROR,
  OVERFLOW_UNDERFLOW,
  NO_DIGITS,
  TRAILING_DIGITS
} StrErr;

typedef enum StrConvOpts {
  NO_OPTS,
  ALLOW_TRAILING
} StrConvOpts;

typedef struct STOLResult {
  long val;
  StrErr error;
} STOLResult;

STOLResult str_to_long(char *str, StrConvOpts opts);
#endif
