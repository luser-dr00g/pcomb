#define PC9PAR_H
#ifndef PC9FP_H
  #include "pc9fp.h"
#endif
#include "ppnarg.h"

enum parser_symbols {
  VALUE = SYM1, PRED, P, PP, NN, Q, R, FF, XX, AA, ID, USE, ATOM,
  SYM2
};

list parse( parser p, list input );

parser result( object a );
parser zero( void );
parser item( void );

parser bind( parser p, oper f );
parser plus( parser p, parser q );
#define PLUS(...)  reduce( plus, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser sat( predicate pred );

parser alpha( void );
parser digit( void );
parser lit( object a );
parser chr( int c );
parser str( char *s );
parser anyof( char *s );
parser noneof( char *s );

parser seq( parser p, parser q );
#define SEQ(...)  reduce( seq, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser xthen( parser p, parser q );
parser thenx( parser p, parser q );
parser into( parser p, object id, parser q );

parser maybe( parser p );
parser forward( void );
parser many( parser p );
parser some( parser p );
parser trim( parser p );

parser using( parser p, fOperator *f );

parser regex( char *re );

int par_main( void );
