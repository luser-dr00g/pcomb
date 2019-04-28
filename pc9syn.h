#define PC9SYN_H
#ifndef PC9TOK_H
  #include "pc9tok.h"
#endif

enum syntax_analysis_symbols {
  func_def = SYM3,
  SYM4
};

list tree_from_tokens( void *s );
