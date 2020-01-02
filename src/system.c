#include "system.h"
#include "memory.h"
#include "interpreter.h"

int system_init(void) {
  memory_init();
  interpreter_init();

  return 0;
}
