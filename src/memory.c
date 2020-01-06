#include "memory.h"
#include "allocator.h"
#include "error.h"
#include "hash.h"

typedef struct did_install_primitive_hooks_s did_install_primitive_hooks_t;
struct did_install_primitive_hooks_s {
  did_install_primitive_func hook;
  did_install_primitive_hooks_t *next;
};

object_t *g_scheme_null;
object_t *g_false;
object_t *g_true;

static did_install_primitive_hooks_t *lg_did_install_primitive_hooks = NULL;

static hash_t *lg_symbol_table;

static allocator_t *lg_object_allocator;
static byte_allocator_t *lg_byte_allocator;
static allocator_t *lg_the_conses;
static allocator_t *lg_the_strings;
static allocator_t *lg_the_symbols;
static allocator_t *lg_the_lambdas;
static allocator_t *lg_the_primitives;
static allocator_t *lg_the_doubles;

#define OBJECT_PAGE_SIZE (1 << 20)
#define BYTES_PAGE_SIZE (1 << 21)
#define STRINGS_PAGE_SIZE (1 << 14)
#define SYMBOLS_PAGE_SIZE (1 << 14)
#define CONS_PAGE_SIZE (1 << 20)
#define LAMBDA_PAGE_SIZE (1 << 14)
#define PRIMITIVE_PAGE_SIZE (1 << 14)
#define DOUBLE_PAGE_SIZE (1 << 14)

int memory_init() {
  g_scheme_null = (object_t*)malloc(sizeof(object_t));
  g_scheme_null->type = SCHEME_NULL;

  lg_object_allocator = make_allocator(sizeof(object_t), OBJECT_PAGE_SIZE);
  lg_byte_allocator = make_byte_allocator(BYTES_PAGE_SIZE);
  lg_the_conses = make_allocator(sizeof(cons_entry_t), CONS_PAGE_SIZE);
  lg_the_strings = make_allocator(sizeof(string_entry_t), STRINGS_PAGE_SIZE);
  lg_the_symbols = make_allocator(sizeof(symbol_entry_t), SYMBOLS_PAGE_SIZE);
  lg_the_lambdas = make_allocator(sizeof(lambda_entry_t), LAMBDA_PAGE_SIZE);
  lg_the_primitives = make_allocator(sizeof(primitive_entry_t), PRIMITIVE_PAGE_SIZE);
  lg_the_doubles = make_allocator(sizeof(double), DOUBLE_PAGE_SIZE);

  lg_symbol_table = make_hash(1<<14);

  g_false = symbol("#f");
  g_true = symbol("#t");

  return 0;
}

void add_did_install_primitive_hook(did_install_primitive_func hook) {
  did_install_primitive_hooks_t *entry = (did_install_primitive_hooks_t*)malloc(sizeof(did_install_primitive_hooks_t));
  entry->hook = hook;
  entry->next = lg_did_install_primitive_hooks;
  lg_did_install_primitive_hooks = entry;
}

static inline object_t *allocate_object() {
  object_t *object = (object_t*)allocator_allocate(lg_object_allocator, NULL);
  ASSERT_OR_ERROR(object != NULL, "Could not allocate object");

  return object;
}

object_t *allocate_string(size_t len, string_entry_t **outentry) {
  object_t *object = allocate_object();
  object->type = SCHEME_STRING;
  char *newstr = (char*)byte_allocator_allocate(lg_byte_allocator, len + 1);
  uint64_t idx;
  string_entry_t *entry = allocator_allocate(lg_the_strings, &idx);
  entry->len = len;
  entry->str = newstr;
  ASSERT_OR_ERROR(idx <= INT64_MAX, "index too big");
  object->number_or_index = (int64_t)idx;

  if (outentry != NULL) *outentry = entry;

  return object;
}

object_t *allocate_symbol(size_t len, symbol_entry_t **outentry) {
  object_t *object = allocate_object();
  object->type = SCHEME_SYMBOL;
  char *newstr = (char*)byte_allocator_allocate(lg_byte_allocator, len + 1);
  uint64_t idx;
  symbol_entry_t *entry = allocator_allocate(lg_the_symbols, &idx);
  entry->len = len;
  entry->sym = newstr;
  ASSERT_OR_ERROR(idx <= INT64_MAX, "index too big");
  object->number_or_index = (int64_t)idx;

  if (outentry != NULL) *outentry = entry;

  return object;
}

object_t *allocate_cons(cons_entry_t **outentry) {
  object_t *object = allocate_object();
  object->type = SCHEME_CONS;
  uint64_t idx;
  cons_entry_t *entry = (cons_entry_t*)allocator_allocate(lg_the_conses, &idx);
  ASSERT_OR_ERROR(idx <= INT64_MAX, "index too big");
  object->number_or_index = (int64_t)idx;

  if (outentry != NULL) *outentry = entry;

  return object;
}

object_t *allocate_lambda(lambda_entry_t **outentry) {
  object_t *object = allocate_object();
  object->type = SCHEME_LAMBDA;
  uint64_t idx;
  lambda_entry_t *entry = (lambda_entry_t*)allocator_allocate(lg_the_lambdas, &idx);
  ASSERT_OR_ERROR(idx <= INT64_MAX, "index too big");
  object->number_or_index = (int64_t)idx;

  if (outentry != NULL) *outentry = entry;

  return object;  
}

object_t *allocate_primitive(const char *name, primitive_func func, primitive_entry_t **outentry) {
  object_t *object = allocate_object();
  object->type = SCHEME_PRIMITIVE;
  uint64_t idx;
  primitive_entry_t *entry = (primitive_entry_t*)allocator_allocate(lg_the_primitives, &idx);
  ASSERT_OR_ERROR(idx <= INT64_MAX, "index too big");
  object->number_or_index = (int64_t)idx;
  entry->name = name;
  entry->func = func;
  if (outentry != NULL) *outentry = entry;

  for (did_install_primitive_hooks_t *hooks = lg_did_install_primitive_hooks; hooks != NULL; hooks = hooks->next) {
    hooks->hook(object, entry);
  }

  return object;
}

object_t *allocate_number(int64_t number) {
  ASSERT_OR_ERROR(number <= SCHEME_INT_MAX, "number too big");
  object_t *object = allocate_object();
  object->type = SCHEME_NUMBER;
  object->number_or_index = number;

  return object;
}

object_t *allocate_double(double number) {
  object_t *object = allocate_object();
  uint64_t idx;
  double *addr = allocator_allocate(lg_the_doubles, &idx);
  *addr = number;
  object->type = SCHEME_DOUBLE;
  ASSERT_OR_ERROR(idx <= INT64_MAX, "index too big");
  object->number_or_index = (int64_t)idx;
  return object;
}

cons_entry_t *get_cons_entry(object_t *cons) {
  ASSERT_OR_ERROR(cons->type == SCHEME_CONS, "Not a pair");
  return allocator_get_item_at_index(lg_the_conses, (uint64_t)(cons->number_or_index));
}

string_entry_t *get_string_entry(object_t *str) {
  ASSERT_OR_ERROR(str->type == SCHEME_STRING, "Not a string");
  return allocator_get_item_at_index(lg_the_strings, (uint64_t)(str->number_or_index));
}

symbol_entry_t *get_symbol_entry(object_t *sym) {
  ASSERT_OR_ERROR(sym->type == SCHEME_SYMBOL, "Not a symbol");
  return allocator_get_item_at_index(lg_the_symbols, (uint64_t)(sym->number_or_index));
}

lambda_entry_t *get_lambda_entry(object_t *lambda) {
  ASSERT_OR_ERROR(lambda->type == SCHEME_LAMBDA, "Not a lambda");
  return allocator_get_item_at_index(lg_the_lambdas, (uint64_t)(lambda->number_or_index));
}

primitive_entry_t *get_primitive_entry(object_t *primitive) {
  ASSERT_OR_ERROR(primitive->type == SCHEME_PRIMITIVE, "Not a primitive");
  return allocator_get_item_at_index(lg_the_primitives, (uint64_t)(primitive->number_or_index));
}

double get_double(object_t *doub) {
  return *(double*)allocator_get_item_at_index(lg_the_doubles, (uint64_t)(doub->number_or_index));
}

object_t *cons(object_t *car, object_t *cdr) {
  cons_entry_t *entry;
  object_t *object = allocate_cons(&entry);
  entry->car = car;
  entry->cdr = cdr;
  return object;
}

object_t *symbol(const char *text) {
  size_t n = strlen(text);
  return symboln(text, n);
}

object_t *symboln(const char *text, size_t len) {
  object_t *sym = (object_t*)hash_get(lg_symbol_table, text, len);
  if (sym != NULL) {
    return sym;
  }

  symbol_entry_t *entry;
  sym = allocate_symbol(len, &entry);
  entry->len = len;
  memcpy(entry->sym, text, len);
  entry->sym[len] = '\0';

  hash_set(lg_symbol_table, text, len, sym);

  return sym;
}

object_t *lambda(object_t *parameters, object_t *body) {
  lambda_entry_t *entry;
  object_t *lambda = allocate_lambda(&entry);
  entry->parameters = parameters;
  entry->body = body;

  return lambda;
}
