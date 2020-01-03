#ifndef SCHEMIN_PRIMITIVES_H
#define SCHEMIN_PRIMITIVES_H
SCHEMIN_PRIMITIVES_H

#include "scheme_types.h"

int primitives_init(void);

typedef object_t * (*primitive_func)(int argc, object_t *argv[]);

#endif
