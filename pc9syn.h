#define PC9SYN_H
#ifndef PC9TOK_H
  #include "pc9tok.h"
#endif

enum syntax_analysis_symbols {
  func_def = SYM3,
  data_def,
  SYM4
};

list tree_from_tokens( object s );
int syn_main();
