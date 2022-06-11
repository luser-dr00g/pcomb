#include "pc11parser.h"
#include <ctype.h>
#include <string.h>

list
parse( parser p, list input ){
  if(  !valid( p ) || !valid( input ) || p->t != PARSER  ) return  NIL_;
  return  p->Parser.f( p->Parser.env, input );
}


static object
success( object v, list input ){
  return  cons( Symbol( OK ), cons( v, input ) );
}

static object
fail( object v, list input ){
  return  cons( Symbol( FAIL ), cons( v, input ) );
}


boolean
is_ok( object result ){
  return  eq_symbol( OK, first( result ) );
}

boolean
not_ok( object result ){
  return  Boolean( ! valid( is_ok( result ) ) );
}


parser
succeeds( void ){
  return  Parser( NIL_, success );
  return  Parser( env( NIL_, 1, Symbol(VALUE), NIL_ ), success );
}

parser
fails( void ){
  return  Parser( NIL_, fail );
}

parser
result_is( object x ){
  return  Parser( env( NIL_, 1, Symbol(VALUE), x ), success );
}


static list
parse_satisfy( object env, list input ){
  predicate pred = assoc_symbol( SATISFY_PRED, env );
  drop( 1, input );
  object item = first( input );
  if(  ! valid( item )  ) return  fail( String( "empty input", 0 ), input );
  return  valid( apply( pred, item ) )
            ? success( item, rest( input ) )
            : fail( LIST( String( "predicate not satisfied", 0 ), pred ), input );
}

parser
satisfy( predicate pred ){
  return  Parser( env( NIL_, 1, Symbol(SATISFY_PRED), pred ), parse_satisfy );
}


static boolean
is_alpha( object v, object it ){
  return  Boolean( it->t == INT && isalpha( it->Int.i ) );
}

parser
alpha( void ){
  return  satisfy( Operator( NIL_, is_alpha ) );
}


static boolean
is_digit( object v, object it ){
  return  Boolean( it->t == INT && isdigit( it->Int.i ) );
}

parser
digit( void ){
  return  satisfy( Operator( NIL_, is_digit ) );
}


static boolean
is_literal( object ex, object it ){
  return  eq( ex, it );
}

parser
literal( object example ){
  return  satisfy( Operator( example, is_literal ) );
}


parser
chr( int c ){
  return  literal( Int( c ) );
}


parser
str( char *s ){
  return  !*s  ? succeeds()  : then( chr( *s ), str( s+1 ) );
}


static boolean
is_range( object bounds, object it ){
  int lo = first( bounds )->Int.i,
      hi = rest( bounds )->Int.i;
  return  Boolean( it->t == INT && lo <= it->Int.i && it->Int.i <= hi );
}

parser
range( int lo, int hi ){
  return  satisfy( Operator( cons( Int( lo ), Int( hi ) ), is_range ) );
}


static boolean
is_anyof( object set, object it ){
  return  Boolean( it->t == INT && strchr( set->String.str, it->Int.i ) != NULL );
}

parser
anyof( char *s ){
  return  satisfy( Operator( String( s, 0 ), is_anyof ) );
}


static boolean
is_noneof( object set, object it ){
  return  Boolean( it->t == INT && strchr( set->String.str, it->Int.i ) == NULL );
}

parser
noneof( char *s ){
  return  satisfy( Operator( String( s, 0 ), is_noneof ) );
}


static object
parse_either( object env, list input ){
  parser p = assoc_symbol( EITHER_P, env );
  object result = parse( p, input );
  if(  valid( is_ok( result ) )  ) return  result;
  parser q = assoc_symbol( EITHER_Q, env );
  return  parse( q, input );
}

parser
either( parser p, parser q ){
  return  Parser( env( NIL_, 2,
                       Symbol(EITHER_P), p,
		       Symbol(EITHER_Q), q ),
                  parse_either );
}

static object
parse_sequence( object env, list input ){
  parser p = assoc_symbol( SEQUENCE_P, env );
  object p_result = parse( p, input );
  if(  valid( not_ok( p_result ) )  ) return  p_result;

  parser q = assoc_symbol( SEQUENCE_Q, env );
  list remainder = rest( rest( p_result ) );
  object q_result = parse( q, remainder );
  if(  valid( not_ok( q_result ) )  ){
    object q_error = first( rest( q_result ) );
    object q_remainder = rest( rest( q_result ) );
    return  fail( LIST( q_error, String( "after", 0), p_result ), q_remainder );
  }

  operator op = assoc_symbol( SEQUENCE_OP, env );
  return  success( op->Operator.f( first( rest( p_result ) ),
                                   first( rest( q_result ) ) ),
                   rest( rest( q_result ) ) );
}

parser
sequence( parser p, parser q, fBinOperator *op ){
  return  Parser( env( NIL_, 3,
                       Symbol(SEQUENCE_P), p,
                       Symbol(SEQUENCE_Q), q,
                       Symbol(SEQUENCE_OP), Operator( NIL_, op ) ),
                  parse_sequence );
}

static object
concat( object l, object r ){
  //puts( "in concat" );
  //print_list( l );
  //print_list( r ), puts("");
  if(  r->t == LIST
    && valid( eq_symbol( VALUE, first( first( r ) ) ) )
    && ! valid( rest( r ) )
    && ! valid( rest( first( r ) ) )  )
    return  l;
  switch(  l->t  ){
  case LIST: return  cons( first( l ), concat( rest( l ), r ) );
  default: return  cons( l, r );
  }
}

parser
then( parser p, parser q ){
  return  sequence( p, q, concat );
}

static object
left( object l, object r ){
  return  l;
}

static object
right( object l, object r ){
  return  r;
}

parser
xthen( parser p, parser q ){
  return  sequence( p, q, right );
}

parser
thenx( parser p, parser q ){
  return  sequence( p, q, left );
}


parser
forward( void ){
  return  Parser( 0, 0 );
}


parser
maybe( parser p ){
  return  either( p, succeeds() );
}

parser
many( parser p ){
  parser q = forward();
  parser r = maybe( then( p, q ) );
  *q = *r;
  return  r;
}

parser
some( parser p ){
  return  then( p, many( p ) );
}


static object
parse_bind( object env, list input ){
  parser p = assoc_symbol( BIND_P, env );
  operator op = assoc_symbol( BIND_OP, env );
  object result = parse( p, input );
  if(  valid( not_ok( result ) )  ) return  result;
  object payload = rest( result ),
         value = first( payload ),
         remainder = rest( payload );
  return  success( apply( (union object[]){{.Operator={
    OPERATOR, append(op->Operator.env, env), op->Operator.f, op->Operator.printname
  }}}, value ), remainder );
}

parser
bind( parser p, operator op ){
  return  Parser( env( NIL_, 2,
                       Symbol(BIND_P), p,
                       Symbol(BIND_OP), op ),
                  parse_bind );
}


static object
parse_into( object v, list input ){
  //puts("in into");
  parser p = assoc_symbol( INTO_P, v );
  object p_result = parse( p, input );
  //print_list( p_result ), puts("");
  if(  valid( not_ok( p_result ) )  ) return  p_result;
  object id = assoc_symbol( INTO_ID, v );
  parser q = assoc_symbol( INTO_Q, v );
  object q_result = q->Parser.f( env( q->Parser.env, 1,
                                      id, first( rest( p_result ) ) ),
		                 rest( rest( p_result ) ) );
  if(  valid( not_ok( q_result ) )  ){
    object q_error = first( rest( q_result ) );
    object q_remainder = rest( rest( q_result ) );
    return  fail( LIST( q_error, String( "after", 0), p_result ), q_remainder );
  }
  return  q_result;
}

parser
into( parser p, object id, parser q ){
  return  Parser( env( NIL_, 3,
                       Symbol(INTO_P), p,
                       Symbol(INTO_ID), id,
                       Symbol(INTO_Q), q ),
                  parse_into );
}

boolean
always_true( object v, object it ){
  return  T_;
}

parser item( void ){
  return  satisfy( Operator( NIL_, always_true ) );
}

static parser
apply_meta( parser a, object it ){
  switch(  it->Int.i  ){
  default:  return  a;
  case '*': return  many( a );
  case '+': return  some( a );
  case '?': return  maybe( a );
  }
}

static parser on_dot( object v, object it ){ return  item(); }
static parser on_chr( object v, object it ){ return  literal( it ); }
static parser on_meta( object v, object it ){
  //puts("in on_meta");
  //print_list( v ), puts("");
  //print_list( it ), puts("");
  parser atom = assoc_symbol( ATOM, v );
  if(  it->t == LIST 
    && valid( eq_symbol( VALUE, first( first( it ) ) ) )
    && ! valid( rest( it ) )
    && ! valid( rest( rest( it ) ) )  )
    return  atom;
  return  apply_meta( atom, it );
}
static parser on_term( object v, object it ){
  //it = first( it );
  //puts( "in on_term" );
  //print_list( v ), puts("");
  //print_list( it ), puts("");
  if(  it->t == LIST  &&  ! valid( rest( it ) )  ) it = first( it ); 
  if(  it->t == PARSER  ) return  it;
  return  collapse( then, it );
}
static parser on_expr( object v, object it ){
  //puts( "in on_expr" );
  //print_list( v ), puts("");
  //print_list( it), puts("");
  if(  it->t == LIST  &&  ! valid( rest( it ) )  ) it = first( it );
  if(  it->t == PARSER  ) return  it;
  return  collapse( either, it );
}

#define META     "*+?"
#define SPECIAL  META ".|()"
static parser
regex_grammar( void ){
  parser dot = bind( chr('.'), Operator( NIL_, on_dot ) );
  parser meta = anyof( META );
  parser escape = xthen( chr('\\'), anyof( SPECIAL "\\" ) );
  parser chr_ = bind( either( escape, noneof( SPECIAL ) ),
                      Operator( NIL_, on_chr ) );
  parser expr_ = forward();
  parser atom = ANY( dot,
                     xthen( chr('('), thenx( expr_, chr(')') ) ),
                     chr_ );
  parser factor = into( atom, Symbol(ATOM),
                        bind( maybe( meta ), Operator( NIL_, on_meta ) ) );
  parser term = bind( some( factor ),
                      Operator( NIL_, on_term ) );
  parser expr = bind( then( term, many( xthen( chr('|'), term ) ) ),
                      Operator( NIL_, on_expr ) );
  *expr_ = *expr;
  return  expr;
}


parser
regex( char *re ){
  static parser p;
  if(  !p  ) p = regex_grammar();
  //print_list( p ), puts("");
  object result = parse( p, chars_from_str( re ) );
  //print_list( result ), puts("");
  if(  valid( not_ok( result ) )  ) return  result;
  return  first( rest( result ) );
}
