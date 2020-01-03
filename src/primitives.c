#include "primitives.h"
#include <assert.h>
#include "memory.h"
#include "error.h"

typedef struct primitive_mapping_s {
  const char *name;
  primitive_func func;
} primitive_mapping_t;

static object_t *car_primitive(int argc, object_t *argv[]) {
  ASSERT_OR_ERROR(argc == 1, "Expected 1 arg");
  ASSERT_OR_ERROR(argv[0]->type == SCHEME_CONS, "Expected cons");
  cons_entry_t *entry = get_cons_entry(argv[0]);
  return entry->car;
}

static primitive_mapping_t primitives[] = {
  {"car", car_primitive}
};

int primitives_init(void) {
  for (uint64_t idx = 0; idx < sizeof(primitives) / sizeof(primitive_mapping_t); idx++) {
    primitive_mapping_t *mapping = &primitives[idx];
    allocate_primitive(mapping->name, mapping->func, NULL);
  }

  return 0;
}
