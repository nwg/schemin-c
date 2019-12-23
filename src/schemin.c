#include <stdio.h>
#include <locale.h>

typedef enum {
  NUMBER,
  STRING,
  SYMBOL,
  CONS
} type_t;

typedef struct const_s cons_t;

typedef union {
  int64_t number;
  const char *str;
  const char *symbol;
  cons_t *cons;
} data_t;

typedef struct object_s {
  type_t type;
  data_t data;
} object_t;

struct cons_s {
  object_t *car;
  object_t *cdr;
};

int read_scheme(const char *exp, cons_t **result) {
   return 0;
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  printf("Here\n");
  return 0;
}
