#include "memory.h"
#include "allocator.h"
#include "error.h"

object_t *g_scheme_null;
object_t *g_scheme_null;

static allocator_t *lg_object_allocator;
static byte_allocator_t *lg_byte_allocator;
static allocator_t *lg_the_conses;
static allocator_t *lg_the_strings;
static allocator_t *lg_the_symbols;

#define OBJECT_PAGE_SIZE (1 << 20)
#define BYTES_PAGE_SIZE (1 << 21)
#define STRINGS_PAGE_SIZE (1 << 14)
#define SYMBOLS_PAGE_SIZE (1 << 14)
#define CONS_PAGE_SIZE (1 << 20)

int memory_init() {
  g_scheme_null = (object_t*)malloc(sizeof(object_t));
  g_scheme_null->type = SCHEME_NULL;

  lg_object_allocator = make_allocator(sizeof(object_t), OBJECT_PAGE_SIZE);
  lg_byte_allocator = make_byte_allocator(BYTES_PAGE_SIZE);
  lg_the_conses = make_allocator(sizeof(cons_entry_t), CONS_PAGE_SIZE);
  lg_the_strings = make_allocator(sizeof(string_entry_t), STRINGS_PAGE_SIZE);
  lg_the_symbols = make_allocator(sizeof(symbol_entry_t), SYMBOLS_PAGE_SIZE);

  return 0;
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
  object->number_or_index = idx;

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
  object->number_or_index = idx;

  if (outentry != NULL) *outentry = entry;

  return object;
}

object_t *allocate_cons(cons_entry_t **outentry) {
  object_t *object = allocate_object();
  object->type = SCHEME_CONS;
  uint64_t idx;
  cons_entry_t *entry = (cons_entry_t*)allocator_allocate(lg_the_conses, &idx);
  object->number_or_index = idx;

  if (outentry != NULL) *outentry = entry;

  return object;
}

cons_entry_t *get_cons_entry(object_t *cons) {
  ASSERT_OR_ERROR(cons->type == SCHEME_CONS, "Not a pair");
  return allocator_get_item_at_index(lg_the_conses, cons->number_or_index);
}

string_entry_t *get_string_entry(object_t *str) {
  ASSERT_OR_ERROR(str->type == SCHEME_STRING, "Not a string");
  return allocator_get_item_at_index(lg_the_strings, str->number_or_index);
}

symbol_entry_t *get_symbol_entry(object_t *sym) {
  ASSERT_OR_ERROR(sym->type == SCHEME_SYMBOL, "Not a symbol");
  return allocator_get_item_at_index(lg_the_symbols, sym->number_or_index);
}
