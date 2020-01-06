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
  return car(argv[0]);
}

static object_t *equal_primitive(int argc, object_t *argv[]) {
  ASSERT_OR_ERROR(argc == 2, "bad argc");
  ASSERT_OR_ERROR(argv[0]->type == SCHEME_NUMBER, "not a number");
  ASSERT_OR_ERROR(argv[1]->type == SCHEME_NUMBER, "not a number");

  if (argv[0]->number_or_index == argv[1]->number_or_index) {
    return g_true;
  }

  return g_false;
}

static object_t *sub_primitive(int argc, object_t *argv[]) {
  for (int i = 0; i < argc; i++) {
  ASSERT_OR_ERROR(argv[i]->type == SCHEME_NUMBER, "not a number");
  }

  object_t *result = allocate_number(argv[0]->number_or_index);
  int64_t value = argv[0]->number_or_index;
  for (int i = 1; i < argc; i++) {
    value -= argv[i]->number_or_index;
  }
  result->number_or_index = value;
  
  return result;
}

static object_t *mul_primitive(int argc, object_t *argv[]) {
  for (int i = 0; i < argc; i++) {
    ASSERT_OR_ERROR(argv[i]->type == SCHEME_NUMBER, "not a number");
  }

  object_t *result = allocate_number(argv[0]->number_or_index);
  uint64_t value = argv[0]->number_or_index;
  for (int i = 1; i < argc; i++) {
    value *= argv[i]->number_or_index;
  }
  result->number_or_index = value;
  
  return result;
}

static primitive_mapping_t primitives[] = {
  {"car", car_primitive},
  {"=", equal_primitive},
  {"-", sub_primitive},
  {"*", mul_primitive}
};

int primitives_init(void) {
  for (uint64_t idx = 0; idx < sizeof(primitives) / sizeof(primitive_mapping_t); idx++) {
    primitive_mapping_t *mapping = &primitives[idx];
    allocate_primitive(mapping->name, mapping->func, NULL);
  }

  return 0;
}
