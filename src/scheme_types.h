#ifndef SCHEMIN_SCHEME_TYPES_H
#define SCHEMIN_SCHEME_TYPES_H
SCHEMIN_SCHEME_TYPES_H

#include <stdint.h>

typedef enum {
  SCHEME_NUMBER,
  SCHEME_STRING,
  SCHEME_SYMBOL,
  SCHEME_CONS,
  SCHEME_NULL,
  SCHEME_LAMBDA,
  SCHEME_PRIMITIVE,
  SCHEME_DOUBLE
} type_t;

typedef struct object_s {
  int64_t number_or_index : 61;
  type_t type : 3;
} object_t;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-macros"

#define SCHEME_INT_MAX ((1LL << 60) - 1)
#define SCHEME_INT_MIN (-(1LL << 60))

#pragma clang diagnostic pop

#endif
