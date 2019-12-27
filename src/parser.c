#include "parser.h"
#include <utf8proc.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "minmax.h"
#include "scheme_types.h"

typedef bool (*codepoint_predicate_func)(utf8proc_int32_t codepoint, void *data);

static size_t utf8_take(const utf8proc_uint8_t *str, utf8proc_size_t lenstr, codepoint_predicate_func p, void *p_data) {
  if (lenstr == 0) {
    return 0;
  }
  
  const utf8proc_uint8_t *ptr = str;
  while (ptr < str + lenstr) {
    utf8proc_int32_t codepoint;
    utf8proc_size_t len;
    len = utf8proc_iterate_unsafe(ptr, &codepoint);
    assert(utf8proc_codepoint_valid(codepoint));
    if (!p(codepoint, p_data)) {
      break;
    }

    ptr += len;
  }

  return (size_t)(ptr - str);
}

static utf8proc_size_t utf8_take_reverse(const utf8proc_uint8_t *str, utf8proc_size_t strlen, codepoint_predicate_func p, void *p_data, const utf8proc_uint8_t **outdst) {
  utf8proc_int32_t codepoint;
  utf8proc_size_t offset = strlen;
  while (offset > 0) {
    utf8proc_size_t len;
    len = utf8proc_iterate_reverse_unsafe(str, offset, &codepoint);
    assert(utf8proc_codepoint_valid(codepoint));
    if (!p(codepoint, p_data)) {
      break;
    }
    offset -= len;
  }

  assert(offset >= 0);
  *outdst = &str[offset];
  return strlen - offset;
}

static bool verify_matching_parens(const char *utf8, size_t len, int *maxdepth) {
  int count = 0;
  int imaxdepth = 0;
  for (int i = 0; i < len; i++) {
    if (utf8[i] == '(') {
      count++;
      imaxdepth = MAX(imaxdepth, count);
    } else if (utf8[i] == ')') {
      count--;
    }

    if (count < 0) return false;
  }

  if (count != 0) return false;

  *maxdepth = imaxdepth;
  return true;
}

static char *strnrchr(const char *string, char c, size_t size) {
  for (size_t i = size - 1; i > 0; i--) {
    if (string[i] == c) {
      return (char*)(&string[i]);
    }
  }

  return NULL;
}

/**
 * @param utf8 a valid sexp
 * return index of string just after closing paren for valid paren sexp in @ref utf8
 */
static size_t scan_for_closing_paren(const char *utf8, size_t len) {
  assert(utf8[0] == '(');
  const char *pos = strnrchr(utf8, ')', len);
  assert(pos != NULL);
  return (size_t)(pos - utf8 + 1);
}

static size_t scan2_for_closing_paren(const char *utf8, size_t len) {
  assert(utf8[0] == '(');
  int count = 1;
  int i;
  for (i = 1; i < len; i++) {
    if (utf8[i] == '(') count++;
    else if (utf8[i] == ')') count--;

    if (count == 0) {
      assert(utf8[i] == ')');
      return (size_t)(i + 1);
    }
  }

  // Should never reach here on valid input. Assert.
  assert(count == 0);
  return (size_t)i;
}

static bool is_whitespace(utf8proc_int32_t codepoint, void *data) {
  (void)data;
  const utf8proc_property_t *p = utf8proc_get_property(codepoint);
  utf8proc_bidi_class_t class = (utf8proc_bidi_class_t)p->bidi_class;
  utf8proc_uint32_t check = (utf8proc_uint32_t)codepoint;
  return (   class == UTF8PROC_BIDI_CLASS_WS
          || check == 0x180E
          || check == 0x200B
          || check == 0x200C
          || check == 0x200D
          || check == 0x2060
          || check == 0xFEFF);
}

#define INVERTED_PREDICATE(name, p) \
  static bool name(utf8proc_int32_t codepoint, void *data) { \
    return !p(codepoint, data); \
  }

INVERTED_PREDICATE(not_is_whitespace, is_whitespace)

static size_t utf8_tok_whitespace(const char *utf8, size_t len, const char **outdst, size_t *outleading) {
  utf8proc_size_t leading = utf8_take((const utf8proc_uint8_t *)utf8, len, &is_whitespace, NULL);
  if (leading == len) {
    *outleading = leading;
    *outdst = NULL;
    return 0;
  }

  utf8proc_uint8_t *start = (utf8proc_uint8_t*)utf8 + leading;
  size_t toksize = utf8_take(start, len - leading, &not_is_whitespace, NULL);

  *outleading = leading;
  *outdst = (const char*)start;
  return toksize;
}

static size_t utf8_tok_lisp(const char *exp, size_t len, const char **outdst, size_t *outleading) {
  utf8proc_size_t leading = utf8_take((const utf8proc_uint8_t *)exp, len, &is_whitespace, NULL);
  if (leading == len) {
    *outleading = leading;
    *outdst = NULL;
    return 0;
  }

  size_t remaining = len - leading;
  utf8proc_uint8_t *start = (utf8proc_uint8_t*)exp + leading;
  if (start[0] == '(') {
    size_t subexp_size = scan2_for_closing_paren((const char*)start, remaining);
    *outdst = (const char*)start;
    *outleading = leading;
    return subexp_size;
  }

  size_t toksize = utf8_take(start, len - leading, &not_is_whitespace, NULL);
  *outleading = leading;
  *outdst = (const char*)start;
  return toksize;
}

static size_t trim_whitespace(const char *utf8, size_t len, const char **dst) {
  utf8proc_size_t leftlen = utf8_take((const utf8proc_uint8_t *)utf8, len, &is_whitespace, NULL);
  const utf8proc_uint8_t *start = (const utf8proc_uint8_t*)utf8 + leftlen;
  if (leftlen == len) {
    *dst = NULL;
    return 0;
  }
  const utf8proc_uint8_t * end;
  utf8_take_reverse((const utf8proc_uint8_t *)utf8, len, &is_whitespace, NULL, &end);

  assert(start <= end);
  *dst = (const char*)start;
  return (size_t)(end - start);
}

static cons_t *valid_sexp_into_cons(const char *sexp, size_t len) {
  if (len == 0)
    return NULL;

  assert(sexp[0] == '(');
  size_t subexp_size = scan2_for_closing_paren(sexp, len) - 2;

  size_t remaining = subexp_size;
  const char *tok;
  size_t leading;
  size_t toklen = utf8_tok_lisp(&sexp[1], subexp_size, &tok, &leading);
  if (tok == NULL)
    return NULL;

  cons_t *result = (cons_t *)malloc(sizeof(cons_t));
  cons_t *current = result;
  while (true) {
    if (result == NULL) result = current;

    object_t *value = valid_exp_into_object(tok, toklen);
    current->car = value;

    remaining -= toklen + leading;
    toklen = utf8_tok_lisp(tok + toklen, remaining, &tok, &leading);

    if (tok == NULL) {
      current->cdr = (object_t*)malloc(sizeof(object_t));
      current->cdr->type = SCHEME_NULL;
      break;
    }
    cons_t *newcons = (cons_t *)malloc(sizeof(cons_t));
    object_t *newobject = (object_t*)malloc(sizeof(object_t));
    newobject->type = SCHEME_CONS;
    newobject->data.cons = newcons;
    current->cdr = newobject;
    current = newcons;
  }

  return result;
}

object_t* valid_exp_into_object(const char *exp, size_t len) {
  len = trim_whitespace(exp, len, &exp);
  if (exp == NULL) return NULL;
  if (len == 0) return NULL;

  object_t *value = (object_t *)malloc(sizeof(object_t));
  if (exp[0] == '(') {
    cons_t *subcons = valid_sexp_into_cons(exp, len);

    if (subcons == NULL) {
      value->type = SCHEME_NULL;
    } else {
      value->type = SCHEME_CONS;
      value->data.cons = subcons;
    }
  } else {
    value->type = SCHEME_SYMBOL;
    char *symbol = (char *)malloc(len + 1);
    memcpy(symbol, exp, len);
    symbol[len] = '\0';
    value->data.symbol = symbol;
  }

  return value;
}
