#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "prettyprint.h"
#include "system.h"

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  if (system_init() != 0) {
    fprintf(stderr, "Could not init system");
    exit(1);
  }

  const char *test = "    (something else (   \"he\\\"re\" we go   ) again)    ";
  size_t test_size = strlen(test);
  if (!quick_verify_scheme(test, test_size)) {
    printf("Invalid scheme\n");
    return 1;
  }

  object_t *result = valid_exp_into_object(test, strlen(test));
  print_object(result);
  printf("\n");
  return 0;
}
