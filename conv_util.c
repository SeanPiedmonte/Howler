#include "conv_util.h"

STOLResult str_to_long(char *str, StrConvOpts opts) {
  STOLResult result;
  char *endptr;
  long val;

  errno = 0;

  val = strtol(str, &endptr, 10);

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
      (errno != 0 && val == 0)) {
    result.val = 0;
    result.error = OVERFLOW_UNDERFLOW;
    return result;
  }

  if (endptr == str) {
    result.val = 0;
    result.error = NO_DIGITS;
    return result;
  }

  if (*endptr != '\0' && !(opts || ALLOW_TRAILING)) {
    result.val = 0;
    result.error = TRAILING_DIGITS;
    return result;
  }

  result.val = val;
  result.error = NO_ERROR;
  return result;
}
