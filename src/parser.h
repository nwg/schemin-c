#ifndef SCHEMIN_PARSER_H
#define SCHEMIN_PARSER_H
SCHEMIN_PARSER_H

#include "scheme_types.h"
#include <stddef.h>

object_t* valid_exp_into_object(const char *exp, size_t len);

#endif
