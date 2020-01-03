#include "interpreter.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "scheme_types.h"
#include "memory.h"
#include "error.h"

static object_t *lg_the_empty_env;
static object_t *lg_global_env;

typedef object_t * (*primitive_func)(int argc, object_t *argv[]);
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
  {"car", car_primitive},
};

static int internal_length(object_t *cons) {
  if (cons == g_scheme_null) {
    return 0;
  }
  ASSERT_OR_ERROR(cons->type == SCHEME_CONS, "length on something not a cons");
  int i = 0;
  while (true) {
    i++;
    cons_entry_t *entry = get_cons_entry(cons);
    if (entry->cdr == g_scheme_null) {
      break;
    }

    ASSERT_OR_ERROR(entry->cdr->type == SCHEME_CONS, "Length on something not a null-terminated list");
    cons = entry->cdr;
  }

  return i;
}

static inline object_t *make_frame(object_t *vars, object_t *vals) {
  if (vars == g_scheme_null) {
    assert(vals == g_scheme_null);
    return cons(g_scheme_null, g_scheme_null);
  }
  assert(vars->type == SCHEME_CONS);
  assert(vals->type == SCHEME_CONS);
  assert(internal_length(vars) == internal_length(vals));
  return cons(vars, vals);
}

static object_t *extend_environment(object_t *vars, object_t *vals, object_t *base_env) {
  ASSERT_OR_ERROR(internal_length(vars) == internal_length(vals), "Vars and vals do not align");
  return cons(make_frame(vars, vals), base_env);
}

static inline bool is_tagged_list(object_t *exp, const char *tag) {
  if (exp->type != SCHEME_CONS) return false;

  cons_entry_t *entry = get_cons_entry(exp);
  if (entry->car->type != SCHEME_SYMBOL) return false;
  symbol_entry_t *sym_entry = get_symbol_entry(entry->car);
  size_t len = strlen(tag);
  if (len != sym_entry->len) return false;
  return strncmp(tag, sym_entry->sym, len) == 0;
}

static inline bool is_definition(object_t *exp) {
  return is_tagged_list(exp, "define");
}

static inline object_t *definition_variable(object_t *exp) {
  return cadr(exp);
}

static inline object_t *definition_value(object_t *exp) {
  return caddr(exp);
}

static inline bool is_quoted(object_t *exp) {
  return is_tagged_list(exp, "quote");
}

static inline object_t *text_of_quotation(object_t *exp) {
  return cadr(exp);
}

static inline bool is_assignment(object_t *exp) {
  return is_tagged_list(exp, "set!");
}

static inline object_t *assignment_variable(object_t *exp) {
  return cadr(exp);
}

static inline object_t *assignment_value(object_t *exp) {
  return caddr(exp);
}

static void add_binding_to_frame(object_t *var, object_t *val, object_t *frame) {
  cons_entry_t *frame_entry = get_cons_entry(frame);

  frame_entry->car = cons(var, frame_entry->car);
  frame_entry->cdr = cons(val, frame_entry->cdr);
}

static inline object_t *first_frame(object_t *env) {
  return get_cons_entry(env)->car;
}

static object_t *scan_frame(object_t *var, object_t *frame, object_t **outvars, object_t **outvals) {
  symbol_entry_t *sentry = get_symbol_entry(var);
  cons_entry_t *centry = get_cons_entry(frame);

  object_t *vars = centry->car;
  object_t *vals = centry->cdr;
  assert(internal_length(vars) == internal_length(vals));

  while (vars != g_scheme_null) {
    cons_entry_t *var_entry = get_cons_entry(vars);
    cons_entry_t *val_entry = get_cons_entry(vals);
    symbol_entry_t *var_sym_entry = get_symbol_entry(var_entry->car);
    if (strncmp(sentry->sym, var_sym_entry->sym, sentry->len) == 0) {
      if (outvars != NULL) *outvars = vars;
      if (outvals != NULL) *outvals = vals;
      return val_entry->car;
    }

    vars = var_entry->cdr;
    vals = val_entry->cdr;
  }

  return NULL;
}

static object_t *define_variable(object_t *var, object_t *val, object_t *env) {
  object_t *frame = first_frame(env);
  object_t *outvals;
  object_t *existing = scan_frame(var, frame, NULL, &outvals);
  if (existing != NULL) {
    cons_entry_t *entry = get_cons_entry(outvals);
    entry->car = val;
    return symbol("ok");
  }

  add_binding_to_frame(var, val, frame);
  return symbol("ok");
}

static object_t *scan_environment(object_t *var, object_t *env, object_t **outvars, object_t **outvals) {
  while (env != lg_the_empty_env) {
    cons_entry_t *entry = get_cons_entry(env);
    object_t *frame = entry->car;
    object_t *result = scan_frame(var, frame, outvars, outvals);
    if (result != NULL) return result;

    env = entry->cdr;
  }

  return NULL;
}

static object_t *setup_env() {
  return extend_environment(g_scheme_null, g_scheme_null, lg_the_empty_env);
}

static object_t *set_variable_value(object_t *var, object_t *val, object_t *env) {
  object_t *vals;
  object_t *existing = scan_environment(var, env, NULL, &vals);
  if (existing == NULL) {
    error("set variable value on non-existent variable");
  }

  cons_entry_t *entry = get_cons_entry(vals);
  entry->car = val;
  return symbol("ok");
}

int interpreter_init(void) {
  lg_the_empty_env = g_scheme_null;
  lg_global_env = setup_env();

  return 0;
}

static inline bool is_self_evaluating(object_t *obj) {
  return obj->type == SCHEME_NULL
      || obj->type == SCHEME_NUMBER
      || obj->type == SCHEME_STRING;
}

static inline bool is_variable(object_t *obj) {
  return obj->type == SCHEME_SYMBOL;
}

static object_t *lookup_variable_value(object_t *name, object_t *env) {
  ASSERT_OR_ERROR(name->type == SCHEME_SYMBOL, "not a symbol");
  return scan_environment(name, env, NULL, NULL);
}

static object_t *eval_with_env(object_t *obj, object_t *env) {
  if (is_self_evaluating(obj)) return obj;
  if (is_variable(obj)) return lookup_variable_value(obj, env);
  if (is_definition(obj)) {
    object_t *variable = definition_variable(obj);
    object_t *value = eval_with_env(definition_value(obj), env);
    return define_variable(variable, value, env);
  }

  if (is_quoted(obj)) {
    return text_of_quotation(obj);
  }

  if (is_assignment(obj)) {
    object_t *variable = assignment_variable(obj);
    object_t *value = eval_with_env(assignment_value(obj), env);
    return set_variable_value(variable, value, env);
  }

  error("Unable to evaluate expression");
}

object_t *eval(object_t *obj) {
  return eval_with_env(obj, lg_global_env);
}
