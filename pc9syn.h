#define PC9SYN_H
#ifndef PC9TOK_H
  #include "pc9tok.h"
#endif

#define Annotations(_) \
 _(func_def) _(data_def) _(type_spec) _(body) _(statement) _(expr) \
 _(decl_list)

enum syntax_analysis_symbols {
  syntax_analysis_symbols = SYM3,
  Annotations( Enum_nam )
  SYM4
};

list tree_from_tokens( object s );
int syn_main();
