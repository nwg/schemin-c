#include "interpreter.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "scheme_types.h"
#include "memory.h"
#include "error.h"

static object_t *lg_the_empty_env;
static object_t *lg_global_env;
static object_t *lg_false;
static object_t *lg_true;

typedef object_t * (*primitive_func)(int argc, object_t *argv[]);
typedef struct primitive_mapping_s {
  const char *name;
  primitive_func func;
} primitive_mapping_t;

static object_t *setup_env(void);

int interpreter_init(void) {
  lg_the_empty_env = g_scheme_null;
  lg_false = symbol("#f");
  lg_true = symbol("#t");
  lg_global_env = setup_env();

  return 0;
}

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

static inline bool is_if(object_t *exp) {
  return is_tagged_list(exp, "if");
}

static inline object_t *if_predicate(object_t *exp) {
  return cadr(exp);
}

static inline object_t *if_consequent(object_t *exp) {
  return caddr(exp);
}

static inline object_t *if_alternative(object_t *exp) {
  return cadddr(exp);
}

static inline bool is_lambda(object_t *exp) {
  return is_tagged_list(exp, "lambda");
}

static inline object_t *lambda_parameters(object_t *exp) {
  return cadr(exp);
}

static inline object_t *lambda_body(object_t *exp) {
  return cddr(exp);
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

#include "prettyprint.h"

static object_t *lookup_variable_value(object_t *name, object_t *env);

static object_t *setup_env(void) {
  object_t *env = extend_environment(g_scheme_null, g_scheme_null, lg_the_empty_env);
  print_object(env);
  printf("\n");
  define_variable(symbol("false"), lg_false, env);
  print_object(lookup_variable_value(symbol("false"), env));
  print_object(env);
  define_variable(symbol("true"), lg_true, env);
  print_object(env);

  return env;
}

static bool is_equal(object_t *obj1, object_t *obj2) {
  if (obj1->type != obj2->type) return false;

  switch (obj1->type) {
    case SCHEME_SYMBOL: {
      return symeq(obj1, obj2);
    }
    case SCHEME_CONS:
    case SCHEME_NULL:
    case SCHEME_NUMBER:
    case SCHEME_STRING:
    case SCHEME_LAMBDA: {
      error("not implemented");
    }
  }
}

static bool is_true(object_t *obj) {
  return !is_equal(obj, lg_false);
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

static bool is_begin(object_t *exp) {
  return is_tagged_list(exp, "begin");
}

static object_t *begin_actions(object_t *exp) {
  return cdr(exp);
}

static bool is_last_exp(object_t *seq) {
  return get_cons_entry(seq)->cdr == g_scheme_null;
}

static object_t *first_exp(object_t *seq) {
  return car(seq);
}

static object_t *rest_exps(object_t *seq) {
  return cdr(seq);
}

static object_t *eval_with_env(object_t *obj, object_t *env);

/*
 * Tail call optimization currently works in clang 11.0.0 for the final eval in this function
 * If you modify this code or use a different compiler, you should check the optimized release assembler
 * output and verify that clang's sibling call optimization is still working as it should
 * Note that modifications to the code with the same logic could cause tail-call optimization pass
 * to fail due to statement reordering.
 * 
 * Tail-call optimization will not work in the code as-written without the memory barrier
 * because of compile-time reordering
 */
static inline object_t *eval_sequence(object_t *seq, object_t *env) {
  object_t *exp = first_exp(seq);
  while (!is_last_exp(seq)) {
    eval_with_env(exp, env);
    seq = rest_exps(seq);
    exp = first_exp(seq);
  }

  __asm__ volatile("" ::: "memory"); // Compile-time memory barrier for tail-call optimization
  return eval_with_env(exp, env);
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

  if (is_if(obj)) {
    object_t *predicate = if_predicate(obj);
    object_t *predicate_result = eval_with_env(predicate, env);
    if (is_true(predicate_result)) {
      object_t *consequent = if_consequent(obj);
      return eval_with_env(consequent, env);
    } else {
      object_t *alternative = if_alternative(obj);
      return eval_with_env(alternative, env);
    }
  }

  if (is_lambda(obj)) {
    object_t *parameters = lambda_parameters(obj);
    object_t *body = lambda_body(obj);
    return lambda(parameters, body);
  }

  if (is_begin(obj)) {
    return eval_sequence(begin_actions(obj), env);
  }

  error("Unable to evaluate expression");
}

object_t *eval(object_t *obj) {
  return eval_with_env(obj, lg_global_env);
}
