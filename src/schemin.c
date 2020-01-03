#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "prettyprint.h"
#include "system.h"
#include "error.h"
#include "interpreter.h"

static const char *statements[] = {
  "(define x (quote abc))",
  "(set! x (quote def))",
  "x"
};

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  
  ASSERT_OR_ERROR(system_init() == 0, "Could not init system");

  for (uint64_t i = 0; i < sizeof(statements) / sizeof(char*); i++) {
    const char *statement = statements[i];
    size_t test_size = strlen(statement);
    if (!quick_verify_scheme(statement, test_size)) {
      error("Invalid scheme\n");
    }

    object_t *exp_object = valid_exp_into_object(statement, strlen(statement));
    object_t *result = eval(exp_object);
    print_object(result);
    printf("\n");
  }
  return 0;
}
