#define PC11PARSER_H
#ifndef PC11OBJECT_H
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
  END_PARSER_SYMBOLS
};

list parse( parser p, list input );

int is_ok( object result );
int not_ok( object result );
parser succeeds( void );
parser fails( void );
parser result_is( object x );
parser satisfy( predicate pred );
parser alpha( void );
parser digit( void );
parser literal( object example );
parser chr( int c );
parser str( char *s );
parser anyof( char *s );
parser noneof( char *s );
parser either( parser p, parser q );
#define ANY(...) reduce( either, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser sequence( parser p, parser q, operator op );
parser xthen( parser p, parser q );
parser  thenx( parser p, parser q );
parser  then( parser p, parser q );
#define SEQ(...) reduce( then, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
parser forward( void );
parser maybe( parser p );
parser many( parser p );
parser some( parser p );
parser item( void );

parser bind( parser p, operator op );
parser into( parser p, object id, parser q );

parser regex( char *re );
list ebnf( char *productions );
