#include "utf8.h"
#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static object_t* valid_exp_into_object(const char *exp, size_t len);

static cons_t *valid_sexp_into_cons(const char *sexp, size_t len) {
  /**
   * "(asd)"
   * "(a (s) d)"
   *
   * drop first char (assert '('))
   * start tokenizing, grab first token
   * begin tokenizer loop
   *   if token ends with ')', note this for later
   *   if first token opens with '(' then recursive call to valid_sexp_into_cons
   * and set car to cons else token is a symbol if token ends with ')' strip
   * that and assign car to symbol else just assign car to symbol if token ended
   * with ')', break out of loop
   */
  printf("in conser\n");

  if (len == 0)
    return NULL;

  assert(sexp[0] == '(');
  size_t subexp_size = scan_for_closing_paren(sexp, len) - 2;

  size_t remaining = subexp_size;
  const char *tok;
  size_t leading;
  size_t toklen = utf8_tok_lisp(&sexp[1], subexp_size, &tok, &leading);
  if (tok == NULL)
    return NULL;

  cons_t *result = (cons_t *)malloc(sizeof(cons_t));
  cons_t *current = result;
  while (true) {
    if (result == NULL) result = current;

    object_t *value = valid_exp_into_object(tok, toklen);
    current->car = value;

    remaining -= toklen + leading;
    toklen = utf8_tok_lisp(tok + toklen, remaining, &tok, &leading);

    if (tok == NULL) {
      current->cdr = (object_t*)malloc(sizeof(object_t));
      current->cdr->type = SCHEME_NULL;
      break;
    }
    cons_t *newcons = (cons_t *)malloc(sizeof(cons_t));
    object_t *newobject = (object_t*)malloc(sizeof(object_t));
    newobject->type = SCHEME_CONS;
    newobject->data.cons = newcons;
    current->cdr = newobject;
    current = newcons;
  }

  return result;
}

static object_t* valid_exp_into_object(const char *exp, size_t len) {
  len = trim_whitespace(exp, len, &exp);
  if (exp == NULL) return NULL;
  if (len == 0) return NULL;

  object_t *value = (object_t *)malloc(sizeof(object_t));
  if (exp[0] == '(') {
    cons_t *subcons = valid_sexp_into_cons(exp, len);

    if (subcons == NULL) {
      value->type = SCHEME_NULL;
    } else {
      value->type = SCHEME_CONS;
      value->data.cons = subcons;
    }
  } else {
    value->type = SCHEME_SYMBOL;
    char *symbol = (char *)malloc(len + 1);
    memcpy(symbol, exp, len);
    symbol[len] = '\0';
    value->data.symbol = symbol;
  }

  return value;
}

static void print_cons(cons_t *cons);

static void print_object(object_t *object) {
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

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  const char *test = "    (something else (   here we go   ) again)    ";
  object_t *result = valid_exp_into_object(test, strlen(test));
  print_object(result);
  printf("\n");
  return 0;
}
