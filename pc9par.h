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

// Unit constructors
parser result( object a );
parser zero( void );
parser item( void );

// Attach operator to process parser results
parser bind( parser p, oper f );

// Logical OR
parser plus( parser p, parser q );
#define PLUS(...)  reduce( plus, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )

// check item with predicate
parser sat( predicate pred );

// characters
parser alpha( void );
parser digit( void );
parser chr( int c );
parser str( char *s );
parser anyof( char *s );
parser noneof( char *s );

// check item against typed object a
parser lit( object a );

// Sequencing
parser seq( parser p, parser q );
#define SEQ(...)  reduce( seq, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser xthen( parser p, parser q );
parser thenx( parser p, parser q );
parser into( parser p, object id, parser q );

// Construct empty parser to fill in later
parser forward( void );

// ? * +
parser maybe( parser p );
parser many( parser p );
parser some( parser p );

// return only first result
parser trim( parser p );

// map results through user callback
parser using( parser p, fOperator *f );

// Construct a parser defined by regular expression
parser regex( char *re );



int par_main( void );
