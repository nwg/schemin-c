#ifndef SCHEMIN_INTERPRETER_H
#define SCHEMIN_INTERPRETER_H
SCHEMIN_INTERPRETER_H
#include <stddef.h>
#include "scheme_types.h"

int interpreter_init(void);
object_t *eval(object_t *obj);

#endif
