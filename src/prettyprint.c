#include "prettyprint.h"
#include <string.h>
#include <stdio.h>

static void print_cons(cons_t *cons);

void print_object(object_t *object) {
  switch (object->type) {
    case SCHEME_CONS: {
      print_cons(object->data.cons);
      break;
    }
    case SCHEME_SYMBOL: {
      printf("%s", object->data.symbol);
      break;
    }
    case SCHEME_NULL: {
      printf("'()");
      break;
    }
    case SCHEME_NUMBER:
    case SCHEME_STRING:
    {
      printf("<not handled>");
      break;
    }
  }
}

static void print_cons(cons_t *cons) {
  printf("(");
  print_object(cons->car);
  printf(" ");
  print_object(cons->cdr);
  printf(")");
}