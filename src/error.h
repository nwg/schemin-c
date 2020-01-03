#ifndef SCHEMIN_ERROR_H
#define SCHEMIN_ERROR_H
SCHEMIN_ERROR_H

#include <stdlib.h>
#include <stdio.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-macros"

_Noreturn static inline void error(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    abort();
    __builtin_unreachable();
}


#define ASSERT_OR_ERROR(cond, msg) \
  if (!(cond)) { \
    error(msg); \
  }

#pragma clang diagnostic pop

#endif
