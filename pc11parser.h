#define PC11PARSER_H
#if ! PC11OBJECT_H
  #include "pc11object.h"
#endif

enum parser_symbol_codes {
  VALUE = END_OBJECT_SYMBOLS,
  OK,
  FAIL,
  SATISFY_PRED,
  EITHER_P,
  EITHER_Q,
  SEQUENCE_P,
  SEQUENCE_Q,
  SEQUENCE_OP,
  BIND_P,
  BIND_OP,
  INTO_P,
  INTO_ID,
  INTO_Q,
  ATOM,
  PROBE_P,
  PROBE_MODE,
  SEQ,
  ANY,
  EPSILON,
  MAYBE,
  MANY,
  END_PARSER_SYMBOLS
};

list parse( parser p, list input );

int is_ok( list result );
int not_ok( list result );
parser succeeds( list result );
parser fails( list errormsg );
parser satisfy( predicate pred );
parser alpha( void );
parser upper( void );
parser lower( void );
parser digit( void );
parser literal( object example );
parser chr( int c );
parser str( char *s );
parser anyof( char *s );
parser noneof( char *s );
parser either( parser p, parser q );
#define ANY(...) reduce( either, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser sequence( parser p, parser q, binoperator op );
parser xthen( parser p, parser q );
parser  thenx( parser p, parser q );
parser  then( parser p, parser q );
#define SEQ(...) reduce( then, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser forward( void );
parser maybe( parser p );
parser many( parser p );
parser some( parser p );
parser item( void );
parser probe( parser p, int mode ); //print on ok iff mode&1, print not ok iff mode&2

parser bind( parser p, operator op );
parser into( parser p, object id, parser q );

// E->T ('|' T)*
// T->F*
// F->A ('*' | '+' | '?')?
// A->'.' | '('E')' | C
// C->S|L|P
// S->'\' ('.' | '|' | '(' | ')' | '[' | ']' | '/' )
// L->'[' '^'? ']'? [^]]* ']'
// P->Plain char
parser regex( char *re );

// D->N '=' E ';'
// N->name
// E->T ('|' T)*
// T->F*
// F->R | N | '[' E ']' | '{' E '}' | '(' E ')' | '/' regex '/'
// R->'"' [^"]* '"' | "'" [^']* "'"
list ebnf( char *productions, list supplements, list handlers );
