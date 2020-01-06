#ifndef SCHEMIN_MEMORY_H
#define SCHEMIN_MEMORY_H
SCHEMIN_MEMORY_H
#include <stdlib.h>
#include <string.h>
#include "scheme_types.h"
#include <stdbool.h>
#include "primitives.h"

extern object_t *g_scheme_null;
extern object_t *g_false;
extern object_t *g_true;

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

typedef struct lambda_entry_s {
  object_t *parameters;
  object_t *body;
} lambda_entry_t;

typedef struct primitive_entry_s {
  const char *name;
  primitive_func func;
} primitive_entry_t;

typedef void (*did_install_primitive_func)(object_t *primitive, primitive_entry_t *entry);

int memory_init(void);
void add_did_install_primitive_hook(did_install_primitive_func hook);
object_t *allocate_cons(cons_entry_t **outentry);
object_t *allocate_symbol(size_t len, symbol_entry_t **outentry);
object_t *allocate_string(size_t len, string_entry_t **outentry);
object_t *allocate_lambda(lambda_entry_t **outentry);
object_t *allocate_primitive(const char *name, primitive_func func, primitive_entry_t **outentry);
object_t *allocate_number(int64_t number);
object_t *allocate_double(double number);

string_entry_t *get_string_entry(object_t *str);
symbol_entry_t *get_symbol_entry(object_t *sym);
cons_entry_t *get_cons_entry(object_t *cons);
lambda_entry_t *get_lambda_entry(object_t *lambda);
primitive_entry_t *get_primitive_entry(object_t *primitive);
double get_double(object_t *doub);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

object_t *cons(object_t *car, object_t *cdr);
object_t *symbol(const char *text);
object_t *symboln(const char *text, size_t len);
object_t *lambda(object_t *parameters, object_t *body);

static inline object_t *car(object_t *cons) {
  return get_cons_entry(cons)->car;
}

static inline object_t *cdr(object_t *cons) {
  return get_cons_entry(cons)->cdr;
}

static inline object_t *cadr(object_t *cons) {
  return get_cons_entry(cdr(cons))->car;
}

static inline object_t *cddr(object_t *cons) {
  return get_cons_entry(cdr(cons))->cdr;
}

static inline object_t *caddr(object_t *cons) {
  return get_cons_entry(cddr(cons))->car;
}

static inline object_t *cdddr(object_t *cons) {
  return cdr(cddr(cons));
}

static inline object_t *cadddr(object_t *cons) {
  return car(cdddr(cons));
}

static inline bool is_eq(object_t *sym1, object_t *sym2) {
  return sym1 == sym2;
}

#pragma clang diagnostic pop

#endif
