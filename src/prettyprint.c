#include "prettyprint.h"
#include "memory.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *new_str_with_word_replaced(const char *s, const char *old,
                                        const char *new) {
  char *result;
  size_t i, cnt = 0;
  size_t newlen = strlen(new);
  size_t oldlen = strlen(old);

  for (i = 0; s[i] != '\0'; i++) {
    if (strstr(&s[i], old) == &s[i]) {
      cnt++;

      i += oldlen - 1;
    }
  }

  result = (char *)malloc(i + cnt * (newlen - oldlen) + 1);

  i = 0;
  while (*s) {
    if (strstr(s, old) == s) {
      strcpy(&result[i], new);
      i += newlen;
      s += oldlen;
    } else {
      result[i++] = *s++;
    }
  }

  result[i] = '\0';
  return result;
}

static void print_cons(object_t *cons);

void print_object(object_t *object) {
  switch (object->type) {
  case SCHEME_CONS: {
    print_cons(object);
    break;
  }
  case SCHEME_SYMBOL: {
    symbol_entry_t *entry = get_symbol_entry(object);
    printf("'%s", entry->sym);
    break;
  }
  case SCHEME_NULL: {
    assert(object == g_scheme_null && "reused null");
    printf("'()");
    break;
  }
  case SCHEME_STRING: {
    string_entry_t *entry = get_string_entry(object);
    const char *escaped = new_str_with_word_replaced(entry->str, "\"", "\\\"");
    printf("\"%s\"", escaped);
    break;
  }
  case SCHEME_NUMBER: {
    printf("<not handled>");
    break;
  }
  }
}

static void print_cons(object_t *cons) {
  cons_entry_t *entry = get_cons_entry(cons);
  printf("(");
  print_object(entry->car);
  printf(" ");
  print_object(entry->cdr);
  printf(")");
}
