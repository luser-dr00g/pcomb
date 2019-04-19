// Syntax analyzer
#include "pc8tok.h"

enum syntax_analysis_symbols {
  func_def = SYM3,

  SYM3_, SYM4 = NEXT10(SYM3_)
};

list tree_from_tokens( void *v );

