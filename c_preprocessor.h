#define C_PREPROCESSOR_H
#if ! PC11PARSER_H
  #include "pc11parser.h"
#endif

enum preprocessor_symbols {
  NEWLINE = END_PARSER_SYMBOLS,
  POS_ROW,
  POS_COL,
  POS_INPUT,
  END_PREPROCESSOR_SYMBOLS
};
