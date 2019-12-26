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

typedef struct cons_s cons_t;

typedef union {
  int64_t number;
  const char *str;
  const char *symbol;
  cons_t *cons;
} data_t;

typedef struct object_s {
  data_t data;
  type_t type;
} object_t;

struct cons_s {
  object_t *car;
  object_t *cdr;
};

#endif
