#include "system.h"
#include "memory.h"
#include "interpreter.h"
#include "primitives.h"

int system_init(void) {
  memory_init();
  interpreter_init();
  primitives_init();

  return 0;
}
