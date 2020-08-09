#define PC9PAR_H
#ifndef PC9FP_H
  #include "pc9fp.h"
#endif
#include "ppnarg.h"

enum parser_symbols {
  VALUE = SYM1, 
  BIND_P, BIND_F, PLUS_P, PLUS_Q, PLUS_R, PLUS_X,
  SAT_PRED, NONEOF_P, LIT_X, SEQ_Q, PREPEND_A,
  INTO_ID, INTO_Q, USING_F, TRIM_P,
  PRED, P, PP, NN, Q, R, FF, XX, AA, ID, USE, ATOM,
  SYM2
};

list parse( parser p, list input );

// Unit constructors
parser result( object a ); // parser that succeeds and returns a
parser zero( void ); // parser that fails
parser item( void ); // parser for one item (char or token)

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
parser chr( int c ); // parser for literal char c
parser str( char *s ); // parser for string s
parser anyof( char *s ); // PLUS( chr(s[0]), ..., chr(s[n]==0) )
parser noneof( char *s ); // not( anyof( s ) )

// check item against typed object a
parser lit( object a );

// Sequencing
parser seq( parser p, parser q ); // p then q
#define SEQ(...)  reduce( seq, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser xthen( parser p, parser q ); // p then q, discard p results
parser thenx( parser p, parser q ); // p then q, discard q results
parser into( parser p, object id, parser q ); // r=parse(p), then parse(q,id=r)

// Construct empty parser to fill in later
parser forward( void );

// ? * +
parser maybe( parser p ); // try parser p 0 or 1 time
parser many( parser p ); // try parser p 0 or more times
parser some( parser p ); // try parser p 1 or more times

// return only first result
parser trim( parser p );

// map results through user callback
parser using( parser p, object v, fOperator *f );

// Construct a parser defined by regular expression
parser regex( char *re );



int par_main( void );
