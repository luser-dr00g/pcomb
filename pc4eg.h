#ifndef PC4_H_
#include "pc4.h"
#endif

typedef struct token token;
struct token {
  enum { ID, INT, OP, STMT } type;
  char *id;
  int    i;
  char  op;
};

typedef struct tree *tree;
struct tree {
  token value;
  tree next;
  tree first;
};
//declare_new_function_for_( tree )


tree parse_program( char *s );
void format_program( tree t );
