#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "prettyprint.h"
#include "system.h"
#include "error.h"
#include "interpreter.h"

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  
  ASSERT_OR_ERROR(system_init() == 0, "Could not init system");

  const char *test = "(define x (quote abc))";
  const char *test2 = "x";
  size_t test_size = strlen(test);
  if (!quick_verify_scheme(test, test_size)) {
    printf("Invalid scheme\n");
    return 1;
  }

  object_t *exp_object = valid_exp_into_object(test, strlen(test));
  object_t *result = eval(exp_object);
  print_object(result);
  printf("\n");
  exp_object = valid_exp_into_object(test2, strlen(test2));
  result = eval(exp_object);
  print_object(result);
  printf("\n");
  return 0;
}
