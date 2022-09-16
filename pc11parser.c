#include "pc11parser.h"
#include <ctype.h>
#include <string.h>

static fParser    success;
static fParser    fail;

static fParser    parse_satisfy;
static fPredicate is_upper;
static fPredicate is_alpha;
static fPredicate is_lower;
static fPredicate is_digit;
static fPredicate is_literal;
static fPredicate is_range;
static fPredicate is_anyof;

static fParser    parse_either;
fBinOperator  either;

static fParser    parse_sequence;

static fBinOperator merge;
fBinOperator  then;

static fBinOperator left;
static fBinOperator right;
fBinOperator  xthen;
fBinOperator  thenx;

static fParser    parse_bind;

static fParser    parse_into;

static fParser    parse_probe;

static fOperator  apply_meta;
static fOperator  on_dot;
static fOperator  on_chr;
static fOperator  on_meta;
static fOperator  on_class;
static fOperator  on_term;
static fOperator  on_expr;

static fOperator  stringify;
static fOperator  symbolize;
static fOperator  encapsulate;

static fOperator  make_matcher;
static fOperator  make_sequence;
static fOperator  make_any;
static fOperator  make_maybe;
static fOperator  make_many;

static fOperator  define_forward;
static fOperator  compile_bnf;
static fOperator  compile_rhs;
static fOperator  define_parser;
static fOperator  wrap_handler;


/* Execute a parser upon an input stream by invoking its function,
   supplying its env. */

list
parse( parser p, list input ){
  if(  !valid( p ) || !valid( input ) || p->t != PARSER  )
    return  cons( Symbol(FAIL),
		  cons( String("parse() validity check failed",0),
			input ) ); 
  return  p->Parser.f( p->Parser.env, input );
}

/*
  The result structure from a parser is either
    ( OK . ( <value> . <remaining input ) )
  or
    ( FAIL . ( <error message> . <remaining input> ) )
*/

static object
success( object result, list input ){
  return  cons( Symbol(OK),
		cons( result,
		      input ) );
}

static object
fail( object errormsg, list input ){
  return  cons( Symbol(FAIL),
		cons( errormsg,
		      input ) );
}


int
is_ok( list result ){
  return  valid( eq_symbol( OK, first( result ) ) );
}

int
not_ok( list result ){
  return  ! is_ok( result );
}


parser
succeeds( list result ){
  return  Parser( result, success );
}

parser
fails( list errormsg ){
  return  Parser( errormsg, fail );
}


/* For all of the parsers after this point, the associated parse_*()
   function should be considered the "lambda" or "closure" function for
   the constructed parser object.
   C, of course, doesn't have lambdas. Hence these closely associated
   functions are close by and have related names.
   These parse_* functions receive an association list of (symbol.value)
   pairs in their env parameter, and they extract their needed values
   using assoc_symbol().
*/

/* The satisfy(pred) parser is the basis for all "leaf" parsers.
   Importantly, it forces the first element off of the (lazy?) input
   list. Therefore, all other functions that operate upon this result
   of this parser need not fuss with suspensions at all. */

parser
satisfy( predicate pred ){
  return  Parser( env( NIL_, 1, Symbol(SATISFY_PRED), pred ), parse_satisfy );
}

static list
parse_satisfy( object env, list input ){
  predicate pred = assoc_symbol( SATISFY_PRED, env );
  drop( 1, input );
  object item = first( input );
  if(  ! valid( item )  ) return  fail( String( "empty input", 0 ),
					input );
  if(  item->t == LIST  )
    item = first( item ); //discard position info
  return  valid( apply( pred, item ) )
            ? success( item,
		       rest( input ) )
            : fail( LIST( String( "predicate not satisfied", 0 ), pred, NIL_ ),
		    input );
}


parser
item( void ){
  return  satisfy( constant( T_ ) );
}


parser
alpha( void ){
  return  satisfy( Predicate( NIL_, is_alpha ) );
}

static boolean
is_alpha( object v, object it ){
  return  Boolean( it->t == INT && isalpha( it->Int.i ) );
}


parser
upper( void ){
  return  satisfy( Predicate( NIL_, is_upper ) );
}

static boolean
is_upper( object v, object it ){
  return  Boolean( it->t == INT && isupper( it->Int.i ) );
}


parser
lower( void ){
  return  satisfy( Predicate( NIL_, is_lower ) );
}

static boolean
is_lower( object v, object it ){
  return  Boolean( it->t == INT && islower( it->Int.i ) );
}


parser
digit( void ){
  return  satisfy( Predicate( NIL_, is_digit ) );
}

static boolean
is_digit( object v, object it ){
  return  Boolean( it->t == INT && isdigit( it->Int.i ) );
}


parser
literal( object example ){
  return  satisfy( Predicate( example, is_literal ) );
}

static boolean
is_literal( object example, object it ){
  return  eq( example, it );
}


parser
chr( int c ){
  return  literal( Int( c ) );
}


parser
str( char *s ){
  return  !*s  ? succeeds( NIL_ )
               : !s[1]  ? chr( *s )
                        : then( chr( *s ), str( s+1 ) );
}



parser
range( int lo, int hi ){
  return  satisfy( Predicate( cons( Int( lo ), Int( hi ) ), is_range ) );
}

static boolean
is_range( object bounds, object it ){
  int lo = first( bounds )->Int.i,
      hi = rest( bounds )->Int.i;
  return  Boolean( it->t == INT && lo <= it->Int.i && it->Int.i <= hi );
}


parser
anyof( char *s ){
  return  satisfy( Predicate( String( s, 0 ), is_anyof ) );
}

static boolean
is_anyof( object set, object it ){
  return  Boolean( it->t == INT && strchr( set->String.str, it->Int.i ) );
}


parser
noneof( char *s ){
  return  satisfy( not( Predicate( String( s, 0 ), is_anyof ) ) );
}


/* The choice combinator. Result is success if either p or q succeed.
   Short circuits q if p was successful. Not lazy. */


parser
either( parser p, parser q ){
  return  Parser( env( NIL_, 2,
		       Symbol(EITHER_Q), q,
                       Symbol(EITHER_P), p ),
                  parse_either );
}

static object
parse_either( object env, list input ){
  parser p = assoc_symbol( EITHER_P, env );
  object result = parse( p, input );
  if(  is_ok( result )  ) return  result;
  parser q = assoc_symbol( EITHER_Q, env );
  return  parse( q, input );
}


/* Sequence 2 parsers and join the 2 results using a binary operator.
   By parameterizing this "joining" operator, this parser supports
   then(), thenx() and xthen() while being completely agnostic as to
   how joining might or might not be done.
 */


parser
sequence( parser p, parser q, binoperator op ){
  return  Parser( env( NIL_, 3,
                       Symbol(SEQUENCE_OP), op,
                       Symbol(SEQUENCE_Q), q,
                       Symbol(SEQUENCE_P), p ),
                  parse_sequence );
}

static object
parse_sequence( object env, list input ){
  parser p = assoc_symbol( SEQUENCE_P, env );
  object p_result = parse( p, input );
  if(  not_ok( p_result )  ) return  p_result;

  parser q = assoc_symbol( SEQUENCE_Q, env );
  list remainder = rest( rest( p_result ) );
  object q_result = parse( q, remainder );
  if(  not_ok( q_result )  ){
    object q_error = first( rest( q_result ) );
    object q_remainder = rest( rest( q_result ) );
    return  fail( LIST( q_error, String( "after", 0),
			first( rest( p_result ) ), NIL_ ),
		  q_remainder );
  }

  binoperator op = assoc_symbol( SEQUENCE_OP, env );
  return  success( op->Operator.f( first( rest( p_result ) ),
                                   first( rest( q_result ) ) ),
                   rest( rest( q_result ) ) );
}


parser
then( parser p, parser q ){
  return  sequence( p, q, BinOperator( merge ) );
}

parser
xthen( parser x, parser q ){
  return  sequence( x, q, BinOperator( right ) );
}

parser
thenx( parser p, parser x ){
  return  sequence( p, x, BinOperator( left ) );
}


/* Some hacking and heuristics to massage 2 objects together into a list,
   taking care if either is already a list */

static object
merge( object left, object right ){
  if(  ! valid( left )  ) return  right;
  if(  right->t == LIST
    && valid( eq_symbol( VALUE, first( first( right ) ) ) )
    && ! valid( rest( right ) )
    && ! valid( rest( first( right ) ) )  )
    return  left;
  switch(  left->t  ){
  case LIST: return  cons( first( left ), merge( rest( left ), right ) );
  default: return  cons( left, right );
  }
}

static object
right( object left, object right ){
  return  right;
}

static object
left( object left, object right ){
  return  left;
}



/* Sequence parsers p and q, but define the value portion of the result of p 
   (if successful) as (id.value) in the env of q.
 */

parser
into( parser p, object id, parser q ){
  return  Parser( env( NIL_, 3,
                       Symbol(INTO_P), p,
                       Symbol(INTO_ID), id,
                       Symbol(INTO_Q), q ),
                  parse_into );
}

static object
parse_into( object v, list input ){
  parser p = assoc_symbol( INTO_P, v );
  object p_result = parse( p, input );
  if(  not_ok( p_result )  ) return  p_result;
  object id = assoc_symbol( INTO_ID, v );
  parser q = assoc_symbol( INTO_Q, v );
  object q_result = q->Parser.f( env( q->Parser.env, 1,
                                      id, first( rest( p_result ) ) ),
		                 rest( rest( p_result ) ) );
  if(  not_ok( q_result )  ){
    object q_error = first( rest( q_result ) );
    object q_remainder = rest( rest( q_result ) );
    return  fail( LIST( q_error, String( "after", 0),
			first( rest( p_result ) ), NIL_ ),
		  q_remainder );
  }
  return  q_result;
}



/* If the parser p succeeds, great! return its result.
   If not, who cares?! call it a success, but give a nothing value.
   If this parser is composed using then(), the merging of values will
   simply ignore this nothing value. It just disappears.
   
   If you bind() this parser to an operator, the operator can test
   if valid( input ) to tell whether p succeeded (and yielded a value)
   or not (which yielded NIL).
 */

parser
maybe( parser p ){
  return  either( p, succeeds( NIL_ ) );
}


/* Uses a forward() to build an infinite sequence of maybe(p). */

parser
many( parser p ){
  parser q = forward();
  *q = *maybe( then( p, q ) );
  return  q;
}


parser
some( parser p ){
  return  then( p, many( p ) );
}



/* Bind transforms a succesful result from the child parser
   through the operator. The operator's environment is supplemented
   with the environment passed to bind itself.
 */

parser
bind( parser p, operator op ){
  return  Parser( env( NIL_, 2,
                       Symbol(BIND_P), p,
                       Symbol(BIND_OP), op ),
                  parse_bind );
}

static object
parse_bind( object env, list input ){
  parser p = assoc_symbol( BIND_P, env );
  operator op = assoc_symbol( BIND_OP, env );
  object result = parse( p, input );
  if(  not_ok( result )  ) return  result;
  object payload = rest( result ),
         value = first( payload ),
         remainder = rest( payload );
  return  success( apply( (union object[]){{.Operator={
    OPERATOR,
    concat(op->Operator.env, env),
    op->Operator.f,
    op->Operator.printname
  }}}, value ), remainder );
}



/* Construct a forwarding parser to aid building of loops.
   This parser can be composed with other parsers.
   Later, the higher level composed parser can be copied over this object 
   to create the point of recursion in the parser graph.
   Remembers the fact that it was created as a forward
   by storing a flag in the hidden allocation record for the parser.
   This flag is not altered by overwriting the parser's normal union object.
 */

parser
forward( void ){
  parser p = Parser( 0, 0 );
  p[-1].Header.is_forward = 1;
  return  p;
}





parser
probe( parser p, int mode ){
  return  Parser( env( NIL_, 2,
		       Symbol(PROBE_MODE), Int( mode ),
		       Symbol(PROBE_P), p ),
		  parse_probe );
}

static object
parse_probe( object env, object input ){
  parser p = assoc_symbol( PROBE_P, env );
  int mode = assoc_symbol( PROBE_MODE, env )->Int.i;
  object result = parse( p, input );
  if(  is_ok( result )  &&  mode & 1  )
    print( result ), puts("");
  else if(  not_ok( result )  &&  mode & 2  )
    print_list( result ), puts("");
  return  result;
}



/* Regex compiler */

static parser regex_grammar( void );
static parser regex_parser;

parser
regex( char *re ){
  if(  ! regex_parser  ) regex_parser = regex_grammar();
  object result = parse( regex_parser, chars_from_str( re ) );
  if(  not_ok( result )  ) return  result;
  return  first( rest( result ) );
}

#define META     "*+?"
#define SPECIAL  META ".|()[]/"

static parser
regex_grammar( void ){
  parser dot       = bind( chr('.'), Operator( NIL_, on_dot ) );
  parser meta      = anyof( META );
  parser escape    = xthen( chr('\\'), anyof( SPECIAL "\\" ) );
  parser class     = xthen( chr('['),
			    thenx( SEQ( maybe( chr('^') ),
			                maybe( chr(']') ),
                                        many( noneof( "]" ) ) ),
			           chr(']') ) );
  parser character = ANY( bind( escape, Operator( NIL_, on_chr ) ),
			  bind( class, Operator( NIL_, on_class ) ),
			  bind( noneof( SPECIAL ), Operator( NIL_, on_chr ) ) );
  parser expr      = forward();
  {
    parser atom    = ANY( dot,
                          xthen( chr('('), thenx( expr, chr(')') ) ),
                          character );
    parser factor  = into( atom, Symbol(REGEX_ATOM),
                           bind( maybe( meta ),
                                 Operator( NIL_, on_meta ) ) );
    parser term    = bind( many( factor ),
                           Operator( NIL_, on_term ) );
    *expr  = *bind( then( term, many( xthen( chr('|'), term ) ) ),
                    Operator( NIL_, on_expr ) );
  }
  return  expr;
}


/* syntax directed compilation to parser */

static parser
apply_meta( parser a, object it ){
  switch(  it->Int.i  ){
  default:  return  a;
  case '*': return  many( a );
  case '+': return  some( a );
  case '?': return  maybe( a );
  }
}

static parser
on_dot( object v, object it ){
  return  item();
}

static parser
on_chr( object v, object it ){
  return  literal( it );
}

static parser
on_meta( object v, object it ){
  parser atom = assoc_symbol( REGEX_ATOM, v );
  if(  it->t == LIST 
    && valid( eq_symbol( VALUE, first( first( it ) ) ) )
    && ! valid( rest( it ) )
    && ! valid( rest( rest( it ) ) )  )
    return  atom;
  return  apply_meta( atom, it );
}

static parser
on_class( object v, object it ){
  if(  first( it )->Int.i == '^'  )
    return  satisfy( not( Predicate( to_string( rest( it ) ), is_anyof ) ) );
  return  satisfy( Predicate( to_string( it ), is_anyof ) );
}

static parser
on_term( object v, object it ){
  if(  ! valid( it )  ) return  NIL_;
  if(  it->t == LIST  &&  ! valid( rest( it ) )  ) it = first( it ); 
  if(  it->t == PARSER  ) return  it;
  return  fold_list( then, it );
}

static parser
on_expr( object v, object it ){
  if(  it->t == LIST  &&  ! valid( rest( it ) )  ) it = first( it );
  if(  it->t == PARSER  ) return  it;
  return  fold_list( either, it );
}



/* EBNF compiler */

static parser ebnf_grammar( void );

/* Compile a block of EBNF definitions into an association list
   of (symbol.parser) pairs.
   Accepts an association list of supplemental parsers for any syntactic
   constructs that are easier to build outside of the EBNF syntax.
   Accepts an association list of operators to bind the results of any
   named parser from the EBNF block or the supplements.
 */

list
ebnf( char *productions, list supplements, list handlers ){
  static parser ebnf_parser;
  if(  !ebnf_parser  ) ebnf_parser = ebnf_grammar();

  object result = parse( ebnf_parser, chars_from_str( productions ) );
  if(  not_ok( result )  ) return  result;

  object payload = first( rest( result ) );
  list defs = concat( payload,
		      env( supplements, 1,
			   Symbol(EBNF_EPSILON), succeeds(NIL_) ) );
  list forwards = map( Operator( NIL_, define_forward ), defs );
  list parsers = map( Operator( forwards, compile_rhs ), defs );
  list final = map( Operator( forwards, define_parser ), parsers );
  map( Operator( forwards, wrap_handler ), handlers );
  return  final;
}

static parser
ebnf_grammar( void ){
  if(  !regex_parser  ) regex_parser = regex_grammar();
  parser spaces = many( anyof( " \t\n" ) );
  parser defining_symbol = thenx( chr( '=' ), spaces );
  parser choice_symbol = thenx( chr( '|' ), spaces );
  parser terminating_symbol = thenx( chr( ';' ), spaces );
  parser name = some( either( anyof( "-_" ), alpha() ) );
  parser identifier = thenx( name, spaces );
  parser terminal =
    bind( 
      thenx( either( thenx( xthen( chr( '"'), many( noneof("\"") ) ),
			    chr( '"') ),
                     thenx( xthen( chr('\''), many( noneof( "'") ) ),
			    chr('\'') ) ),
             spaces ),
      Operator( NIL_, make_matcher ) );
  parser symb = bind( identifier, Operator( NIL_, symbolize ) );
  parser nonterminal = symb;
  parser expr = forward();
  {
    parser factor = ANY( terminal,
			 nonterminal,
			 bind( xthen( then( chr( '[' ), spaces ),
				      thenx( expr,
					     then( chr( ']' ), spaces ) ) ),
			       Operator( NIL_, make_maybe ) ),
			 bind( xthen( then( chr( '{' ), spaces ),
                                      thenx( expr,
					     then( chr( '}' ), spaces ) ) ),
                               Operator( NIL_, make_many ) ),
                         bind( xthen( then( chr( '(' ), spaces ),
				      thenx( expr,
				             then( chr( ')' ), spaces ) ) ),
                               Operator( NIL_, encapsulate ) ),
                         bind( xthen( chr( '/' ),
                                      thenx( regex_parser, chr( '/' ) ) ),
                               Operator( NIL_, encapsulate ) ) );
    parser term = bind( many( factor ), Operator( NIL_, make_sequence ) );
    *expr = *bind( then( term, many( xthen( choice_symbol, term ) ) ),
		   Operator( NIL_, make_any ) );
  };
  parser definition = bind( then( symb,
			    xthen( defining_symbol,
				   thenx( expr, terminating_symbol ) ) ),
			    Operator( NIL_, encapsulate) );
  return  some( definition );
}


/* helpers */

static string
stringify( object env, object input ){
  return  to_string( input );
}

static symbol
symbolize( object env, object input ){
  return  symbol_from_string( to_string( input ) );
}

static list
encapsulate( object env, object input ){
  return  one( input );
}


/* syntax directed translation to list form */

static parser
make_matcher( object env, object input ){
  return  str( to_string( input )->String.str );
}

static list
make_sequence( object env, object input ){
  if(  length( input ) == 0  ) return  Symbol(EBNF_EPSILON);
  if(  length( input ) < 2  ) return  input;
  return  one( cons( Symbol(EBNF_SEQ), input ) );
}

static list
make_any( object env, object input ){
  if(  length( input ) < 2  ) return  input;
  return  one( cons( Symbol(EBNF_ANY), input ) );
}

static list
make_maybe( object env, object input ){
  return  one( cons( Symbol(EBNF_MAYBE), input ) );
}

static list
make_many( object env, object input ){
  return  one( cons( Symbol(EBNF_MANY), input ) );
}


/* stages of constructing the parsers from list form */

static list
define_forward( object env, object it ){
  if(  rest( it )->t == PARSER  ) return  it;
  return  cons( first( it ), forward() );
}

static parser
compile_bnf( object env, object it ){
  operator self = (union object[]){{.Operator={OPERATOR,env,compile_bnf}}};
  switch(  it->t  ){
  default:
    return  it;
  case SYMBOL: {
    object ob = assoc( it, env );
    return  valid( ob )  ? ob  : it;
  }
  case LIST:   {
    object f = first( it );
    if(  valid( eq_symbol( EBNF_SEQ, f ) )  )
      return  fold_list( then,
			map( self, rest( it ) ) );
    if(  valid( eq_symbol( EBNF_ANY, f ) )  )
      return  fold_list( either,
			map( self, rest( it ) ) );
    if(  valid( eq_symbol( EBNF_MANY, f ) )  )
      return  many( map( self, rest( it ) ) );
    if(  valid( eq_symbol( EBNF_MAYBE, f ) )  )
      return  maybe( map( self, rest( it ) ) );
    if(  length( it ) == 1  )
      return  compile_bnf( env, f );
    return  map( self, it );
  }
  }
}

static list
compile_rhs( object env, object it ){
  if(  rest( it )->t == PARSER  ) return  it;
  object result = cons( first( it ),
      map( (union object[]){{.Operator={OPERATOR,env,compile_bnf}}},
	   rest( it ) ) );
  return  result;
}

static list
define_parser( object env, object it ){
  object lhs = assoc( first( it ), env );
  if(  valid( lhs ) && lhs->t == PARSER && lhs->Parser.f == NULL  ){
    object rhs = rest( it );
    if(  rhs->t == LIST  ) rhs = first( rhs );
    *lhs = *rhs;
    return  cons( first( it ), rhs );
  }
  return  it;
}

static list
wrap_handler( object env, object it ){
  object lhs = assoc( first( it ), env );
  if(  valid( lhs ) && lhs->t == PARSER  ){
    object op = rest( it );
    parser copy = Parser( 0, 0 );
    *copy = *lhs;
    *lhs = *bind( copy, op );
  }
  return  it;
}
