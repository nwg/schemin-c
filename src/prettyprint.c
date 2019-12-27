#include "prettyprint.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 

static char *new_str_with_word_replaced(const char *s, const char *old, 
								const char *new) 
{ 
	char *result; 
	int i, cnt = 0; 
	int newlen = strlen(new); 
	int oldlen = strlen(old); 

	for (i = 0; s[i] != '\0'; i++) 
	{ 
		if (strstr(&s[i], old) == &s[i]) 
		{ 
			cnt++; 

			i += oldlen - 1; 
		} 
	} 

	result = (char *)malloc(i + cnt * (newlen - oldlen) + 1); 

	i = 0; 
	while (*s) 
	{ 
		if (strstr(s, old) == s) 
		{ 
			strcpy(&result[i], new); 
			i += newlen; 
			s += oldlen; 
		} 
		else
			result[i++] = *s++; 
	} 

	result[i] = '\0'; 
	return result; 
} 

static void print_cons(cons_t *cons);

void print_object(object_t *object) {
  switch (object->type) {
    case SCHEME_CONS: {
      print_cons(object->data.cons);
      break;
    }
    case SCHEME_SYMBOL: {
      printf("%s", object->data.symbol);
      break;
    }
    case SCHEME_NULL: {
      printf("'()");
      break;
    }
    case SCHEME_STRING: {
      const char *escaped = new_str_with_word_replaced(object->data.str, "\"", "\\\"");
      printf("\"%s\"", escaped);
      break;
    }
    case SCHEME_NUMBER:
    {
      printf("<not handled>");
      break;
    }
  }
}

static void print_cons(cons_t *cons) {
  printf("(");
  print_object(cons->car);
  printf(" ");
  print_object(cons->cdr);
  printf(")");
}