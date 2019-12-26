#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "prettyprint.h"

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  const char *test = "    (something else (   here we go   ) again)    ";
  object_t *result = valid_exp_into_object(test, strlen(test));
  print_object(result);
  printf("\n");
  return 0;
}
