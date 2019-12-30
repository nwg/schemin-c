#include "system.h"
#include "memory.h"

int system_init(void) {
  memory_init();

  return 0;
}