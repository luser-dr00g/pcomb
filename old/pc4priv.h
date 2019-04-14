#ifndef PC4_H_
#include "pc4.h"
#endif
#include <stdlib.h>

typedef result (*handler)( parser, char *);

struct parser { 
  handler test; 
  parser a, b; 
  int c; 
  callback f;
  void *payload;
}; 

#define Types(_) \
  _(parser)      \
  _(result)
#define declare_new_function_for_(type) \
  type new_##type ( struct type input );
Types( declare_new_function_for_ )

#define define_new_function_for_(type)                            \
  type new_##type ( struct type input ){                   \
    type local = calloc( sizeof *local, 1 );               \
    return  local  ?  ( *local = input ), local  :  local; \
  }
