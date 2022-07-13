#define PC11IO_H
#if  ! PC11PARSER_H
  #include "pc11parser.h"
#endif

enum io_symbol_codes {
  ARGS = END_PARSER_SYMBOLS,
  END_IO_SYMBOLS
};

/* Subset of printf() supporting directives
     %%
     %c
     %s
 */
int pprintf( char const *fmt, ... );

/* Subset of scanf() supporting directives
     %%
     %c
     %s
 */
int pscanf( char const *fmt, ... );
