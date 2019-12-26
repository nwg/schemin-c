#ifndef SCHEMIN_UTF8_H
#define SCHEMIN_UTF8_H
SCHEMIN_UTF8_H
#include <sys/types.h>
#include <stdbool.h>

size_t utf8_tok_whitespace(const char *utf8, size_t len, const char **outdst, size_t *outleading);
size_t utf8_tok_lisp(const char *exp, size_t len, const char **outdst, size_t *outleading);
bool verify_matching_parens(const char *utf8, size_t len, int *maxdepth);
size_t scan_for_closing_paren(const char *utf8, size_t len);
size_t trim_whitespace(const char *utf8, size_t strlen, const char **dst);

#endif
