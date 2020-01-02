#ifndef SCHEMIN_MEMORY_H
#define SCHEMIN_MEMORY_H
SCHEMIN_MEMORY_H
#include <stdlib.h>
#include <string.h>
#include "scheme_types.h"

extern object_t *g_scheme_null;

typedef struct string_entry_s {
  char *str;
  size_t len;
} string_entry_t;

typedef struct symbol_entry_s {
  char *sym;
  size_t len;
} symbol_entry_t;

typedef struct cons_entry_s {
  object_t *car;
  object_t *cdr;
} cons_entry_t;

int memory_init(void);
object_t *allocate_cons(cons_entry_t **outentry);
object_t *allocate_symbol(size_t len, symbol_entry_t **outentry);
object_t *allocate_string(size_t len, string_entry_t **outentry);

string_entry_t *get_string_entry(object_t *str);
symbol_entry_t *get_symbol_entry(object_t *sym);
cons_entry_t *get_cons_entry(object_t *cons);

#endif
