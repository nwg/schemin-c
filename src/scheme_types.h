#ifndef SCHEMIN_SCHEME_TYPES_H
#define SCHEMIN_SCHEME_TYPES_H
SCHEMIN_SCHEME_TYPES_H

#include <stdint.h>

typedef enum {
  SCHEME_NUMBER,
  SCHEME_STRING,
  SCHEME_SYMBOL,
  SCHEME_CONS,
  SCHEME_NULL
} type_t;

typedef struct object_s {
  uint64_t number_or_index : 61;
  type_t type : 3;
} object_t;

#endif
