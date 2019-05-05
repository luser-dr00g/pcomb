//pc8par.h Parser Combinators
#include "pc8fp.h"
#include "ppnarg.h"

enum pc_symbols {
  VALUE = SYM1,
  PRED, P, Q, F, X, A, ID, USE, ATOM,
  SYM1_, SYM2 = NEXT10(SYM1_)
};

list parse( parser p, list input );

parser result( object a );
parser zero( void );
parser item( void );

parser bind( parser p, operator f );
parser plus( parser p, parser q );
#define PLUS(...) reduce( PP_NARG(__VA_ARGS__), plus, (object[]){ __VA_ARGS__ } )
parser sat( predicate pred );

parser alpha( void );
parser digit( void );
parser lit( object a );
parser chr( int c );
parser str( char *s );
parser anyof( char *s );
parser noneof( char *s );

parser seq( parser p, parser q );
#define SEQ(...) reduce( PP_NARG(__VA_ARGS__), seq, (object[]){ __VA_ARGS__ } )
parser xthen( parser _, parser q );
parser thenx( parser p, parser _ );
parser into( parser p, object id, parser q );

parser maybe( parser p );
parser forward( void );
parser many( parser p );
parser some( parser p );
parser trim( parser p );

parser using( parser p, fOperator *f );

parser regex( char *re );
