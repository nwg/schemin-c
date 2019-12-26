#include <utf8proc.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "utf8.h"
#include "minmax.h"

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

static const utf8proc_uint8_t * utf8_take_reverse(const utf8proc_uint8_t *str, utf8proc_size_t strlen, codepoint_predicate_func p, void *p_data) {
  utf8proc_int32_t codepoint;
  utf8proc_size_t offset = strlen;
  while (offset > 0) {
    utf8proc_size_t len;
    len = utf8proc_iterate_reverse_unsafe(str, offset, &codepoint);
    assert(utf8proc_codepoint_valid(codepoint));
    offset -= len;
    if (!p(codepoint, p_data)) {
      break;
    }
  }

  assert(offset >= 0);
  return &str[offset];
}

bool verify_matching_parens(const char *utf8, size_t len, int *maxdepth) {
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
  for (unsigned int i = size - 1; size > 0; size--) {
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
size_t scan_for_closing_paren(const char *utf8, size_t len) {
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

size_t utf8_tok_whitespace(const char *utf8, size_t len, const char **outdst, size_t *outleading) {
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

size_t utf8_tok_lisp(const char *exp, size_t len, const char **outdst, size_t *outleading) {
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

size_t trim_whitespace(const char *utf8, size_t len, const char **dst) {
  utf8proc_size_t leftlen = utf8_take((const utf8proc_uint8_t *)utf8, len, &is_whitespace, NULL);
  const utf8proc_uint8_t *start = (const utf8proc_uint8_t*)utf8 + leftlen;
  if (leftlen == len) {
    *dst = NULL;
    return 0;
  }
  const utf8proc_uint8_t * end = utf8_take_reverse((const utf8proc_uint8_t *)utf8, len, &is_whitespace, NULL);

  assert(start <= end);
  *dst = (const char*)start;
  return (size_t)(end - start + 1);
}

