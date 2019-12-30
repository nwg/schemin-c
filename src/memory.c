#include "memory.h"

object_t *g_scheme_null;

int memory_init() {
  g_scheme_null = (object_t*)malloc(sizeof(object_t));
  g_scheme_null->type = SCHEME_NULL;

  return 0;
}

