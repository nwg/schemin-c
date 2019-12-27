#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "prettyprint.h"

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  const char *test = "    (something else (   here we go   ) again)    ";
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
