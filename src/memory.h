#ifndef SCHEMIN_MEMORY_H
#define SCHEMIN_MEMORY_H
SCHEMIN_MEMORY_H
#include <stdlib.h>
#include "scheme_types.h"

extern object_t *g_scheme_null;

int memory_init(void);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

static inline object_t* allocate_object() {
  object_t *object = (object_t*)malloc(sizeof(object_t));
  if (object == NULL) {
    return NULL;
  }

  return object;
}

#pragma clang diagnostic pop

#endif
