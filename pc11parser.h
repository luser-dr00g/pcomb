#define PC11PARSER_H
#if  ! PC11OBJECT_H
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
  REGEX_ATOM,
  PROBE_P,
  PROBE_MODE,
  EBNF_SEQ,
  EBNF_ANY,
  EBNF_EPSILON,
  EBNF_MAYBE,
  EBNF_MANY,
  END_PARSER_SYMBOLS
};


/* Parse the input using parser p. */

list    parse( parser p,
	       list input );


/* Check result from parse(). */

int     is_ok( list result );

int     not_ok( list result );


/* Return OK or FAIL result. */

parser  succeeds( list result );

parser  fails( list errormsg );


/* Emit debugging output from p. Print on ok iff mode&1; print not ok iff mode&2. */

parser  probe( parser p,
	       int mode );


/* The basic (leaf) parser. */

parser  satisfy( predicate pred );


/* Simple parsers built with satisfy(). */

parser  alpha( void );
parser  upper( void );
parser  lower( void );
parser  digit( void );
parser  literal( object example );
parser  chr( int c );
parser  str( char *s );
parser  anyof( char *s );
parser  noneof( char *s );

/* Accept any single element off the input list. */

parser  item( void );



/* Choice ("OR" branches) */


/* Combine 2 parsers into a choice. */

parser  either( parser p,
		parser q );


/* Combine N parsers into a choice. */

#define ANY(...)                    \
  reduce( either,                   \
          PP_NARG(__VA_ARGS__),     \
          (object[]){ __VA_ARGS__ } )



/* Sequence ("AND" branches) */


/* Combine 2 parsers into a sequence, using op to merge the value portions of results. */

parser  sequence( parser p,
		  parser q,
		  binoperator op );


/* Sequence 2 parsers but drop result from first. */

parser  xthen( parser p,
	       parser q );


/* Sequence 2 parsers but drop result from second. */

parser   thenx( parser p,
		parser q );


/* Sequence 2 parsers and concatenate results. */

parser   then( parser p,
	       parser q );


/* Sequence N parsers and concatenate results. */

#define SEQ(...)                    \
  reduce( then,                     \
          PP_NARG(__VA_ARGS__),     \
          (object[]){ __VA_ARGS__ } )


/* Sequence 2 parsers, but pass result from first as a (id.value) pair in second's env. */

parser  into( parser p,
	      object id,
	      parser q );



/* Repetitions */


/* Accept 0 or 1 successful results from p. */

parser  maybe( parser p );


/* Accept 0 or more successful results from p. */

parser  many( parser p );


/* Accept 1 or more successful results from p. */

parser  some( parser p );



/* Transform of values */


/* Process succesful result from p by transforming the value portion with op. */

parser  bind( parser p,
	      operator op );



/* Building recursive parsers */


/* Create an empty parser, useful for building loops.
   A forward declaration of a parser. */

parser  forward( void );



/* Compilers */


/* Compile a regular expression into a parser. */
// E->T ('|' T)*
// T->F*
// F->A ('*' | '+' | '?')?
// A->'.' | '('E')' | C
// C->S|L|P
// S->'\' ('.' | '|' | '(' | ')' | '[' | ']' | '/' )
// L->'[' '^'? ']'? [^]]* ']'
// P->Plain char

parser  regex( char *re );


/* Compile a block of EBNF definitions into a list of (symbol.parser) pairs. */
// D->N '=' E ';'
// N->name
// E->T ('|' T)*
// T->F*
// F->R | N | '[' E ']' | '{' E '}' | '(' E ')' | '/' regex '/'
// R->'"' [^"]* '"' | "'" [^']* "'"

list    ebnf( char *productions,
	      list supplements,
	      list handlers );
