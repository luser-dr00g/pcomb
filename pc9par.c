#include <ctype.h>
#include "pc9par.h"
#include "pc9objpriv.h"


list
parse( parser p, list input ){
  return  valid( p ) && p->t == PARSER && valid( input ) ? p->Parser.f( p->Parser.v, input )  : NIL_;
}


static list
presult( void *v, list input ){
  return  one( cons( assoc( Symbol(VALUE), v ), input ) );
}
parser
result( object a ){
  return  Parser( env( 0, 1, Symbol(VALUE), a ), presult );
}

static list
pzero( void *v, list input ){
  return  NIL_;
}
parser
zero( void ){
  return  Parser( 0, pzero );
}

static list
pitem( void *v, list input ){
  return  valid( input ) ? one( cons( x_( take( 1, input ) ), xs_( input ) ) )  : NIL_;  //strict
  return  valid( input ) ? one( cons( x_( input ), xs_( input ) ) )  : NIL_;             //lazy
}
parser
item( void ){
  return  Parser( 0, pitem );
}

/*
static list
cbind( void *v ){
  oper f = assoc( Symbol(FF), v );
  list r = assoc( Symbol(R), v );
  *r = *at_( r );
  return  valid( r )  ? join( map( Operator( valid( f->Operator.v ) ?
                                             append( copy( f->Operator.v ), v )  : v,
                                             f->Operator.f ),
                                   r) )
                      : NIL_;
}
*/
static list
pbind( void *v, list input ){
  //PRINT( (object)v );
  parser p = assoc( Symbol(P), v );
  oper f = assoc( Symbol(FF), v );
  //PRINT( cons( p, f ) );
  list r = parse( p, input );
  //PRINT( r );
  return  valid( r )  ?
              //r->t == SUSPENSION  ? Suspension( env( 0, 1, Symbol(R), r, Symbol(FF), f  ), cbind )  :
                                    join( map( Operator( valid( f->Operator.v ) ?
                                                         append( copy( f->Operator.v ), v )  : v,
                                                         f->Operator.f ),
                                               r) )
                      : NIL_;
}
parser
bind( parser p, oper f ){
  //PRINT( cons( p, f ) );
  return  Parser( env( 0, 2, Symbol(P), p, Symbol(FF), f ), pbind );
}

static list
bplus( void *v ){
  list r = assoc( Symbol(R), v );
  object qq = assoc( Symbol(Q), v );
  *r = *at_( r );
  return  valid( r )  ? append( r, qq ) : qq;
}
static list
cplus( void *v ){
  parser q = assoc( Symbol(Q), v );
  list input = assoc( Symbol(X), v );
  return  parse( q, input );
}
static list
pplus( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  parser q = assoc( Symbol(Q), v );
  list r = parse( p, input );
  //PRINT( r );
  object qq = Suspension( env( 0, 2, Symbol(Q), q, Symbol(X), input ), cplus );
  return  valid( r )  ? 
              r->t == SUSPENSION  ? Suspension( env( 0, 2, Symbol(R), r, Symbol(Q), qq ), bplus )
                                  : append( r, qq )
                      : qq;
}
parser
plus( parser p, parser q ){
  if(  !q  ) return  p;
  return  Parser( env( 0, 2, Symbol(P), p, Symbol(Q), q ), pplus );
}

static list
psat( void *v, list input ){
  predicate pred = assoc( Symbol(PRED), v );
  object r = apply( pred, x_( input ) );
  return  valid( r )  ? one( cons( x_( input ), xs_( input ) ) )  : NIL_;
}
parser
sat( predicate pred ){
  return  bind( item(), Operator( env( 0, 1, Symbol(PRED), pred ), psat ) );
}

static boolean
palpha( void *v, object o ){
  return  isalpha( o->Int.i )  ? T_  : NIL_;
}
parser
alpha( void ){
  return  sat( Operator( 0, palpha ) );
}

static boolean
pdigit( void *v, object o ){
  return  isdigit( o->Int.i )  ? T_  : NIL_;
}
parser
digit( void ){
  return  sat( Operator( 0, pdigit ) );
}

static boolean
plit( void *v, object o ){
  object a = assoc( Symbol(X), v );
  return  eq( a, o );
}
parser
lit( object a ){
  return  sat( Operator( env( 0, 1, Symbol(X), a ), plit ) );
}

parser
chr( int c ){
  return  lit( Int( c ) );
}

parser
str( char *s ){
  return  *s  ? seq( chr( *s ), str( s+1 ) )  : result(0);
}

parser
anyof( char *s ){
  return  *s  ? plus( chr( *s ), anyof( s+1 ) )  : zero();
}

static list
pnone( void *v, list input ){
  parser p = assoc( Symbol(NN), v );
  object r = parse( p, input );
  *r = *at_( r );
  return  valid( r )  ? NIL_  : pitem( 0, input );
}
parser
noneof( char *s ){
  return  Parser( env( 0, 1, Symbol(NN), anyof( s ) ), pnone );
}


static list
pprepend( void *v, list o ){
  object a = assoc( Symbol(AA), v );
  return  valid( a )  ? cons( cons( a, x_( o ) ), xs_( o ) )  : o;
}
static list
prepend( list a, list b ){
  return  map( Operator( env( 0, 1, Symbol(AA), a ), pprepend ), b );
}
static list
pseq( void *v, list output ){
  parser q = assoc( Symbol(Q), v );
  return  prepend( x_( output ), parse( q, xs_( output ) ) );
}
parser
seq( parser p, parser q ){
  if(  !q  ) return  p;
  return  bind( p, Operator( env( 0, 1, Symbol(Q), q ), pseq ) );
}

static list
pxthen( void *v, list o ){
  return  one( cons( xs_( x_( o ) ), xs_( o ) ) );
}
parser
xthen( parser p, parser q ){
  return  bind( seq( p, q ), Operator( 0, pxthen ) );
}

static list
pthenx( void *v, list o ){
  return  one( cons( x_( x_( o ) ), xs_( o ) ) );
}
parser
thenx( parser p, parser q ){
  return  bind( seq( p, q ), Operator( 0, pthenx ) );
}

static list
pinto( void *v, list o ){
  object id = assoc( Symbol(ID), v );
  parser q = assoc( Symbol(Q), v );
  return  parse( Parser( env( q->Parser.v, 1, id, x_( o ) ), q->Parser.f ), xs_( o ) );
}
parser
into( parser p, object id, parser q ){
  return  bind( p, Operator( env( 0, 2, Symbol(ID), id, Symbol(Q), q ), pinto ) );
}


parser
maybe( parser p ){
  return  plus( p, result(0) );
}

parser
forward( void ){
  return  Parser( 0, 0 );
}

parser
many( parser p ){
  parser q = forward();
  parser r = maybe( seq( p, q ) );
  *q = *r;
  return  r;
}

parser
some( parser p ){
  return  seq( p, many( p ) );
}

static list
ptrim( void *v, list input ){
  parser p = assoc( Symbol(PP), v );
  list r = parse( p, input );
  return  valid( r )  ? one( x_( take( 1, r ) ) )  : r;
}
parser
trim( parser p ){
  return  Parser( env( 0, 1, Symbol(PP), p ), ptrim );
}


static list
pusing( void *v, list o ){
  oper f = assoc( Symbol(USE), v );
  //f->Operator.v = v;
  //PRINT( o );
  return  one( cons( apply( f, x_( o ) ), xs_( o ) ) );
}
parser
using( parser p, fOperator *f ){
  return  bind( p, Operator( env( 0, 1, Symbol(USE), Operator( 0, f ) ), pusing ) );
}


static parser
do_meta( parser a, object o ){
  switch( o->Int.i ){
  case '*':  return  many( a );
  case '+':  return  some( a );
  case '?':  return  maybe( a );
  }
}
static parser
on_meta( void *v, object o ){
  parser atom = assoc( Symbol(ATOM), v );
  return  valid( o ) ? do_meta( atom, o )  : atom;
}
static parser  on_dot( void *v, object o ){ return  item(); }
static parser  on_chr( void *v, object o ){ return  lit( o ); }
static parser  on_term( void *v, object o ){ return  collapse( seq, o ); }
static parser  on_expr( void *v, object o ){ return  collapse( plus, o ); }

#define META     "*+?"
#define SPECIAL  META ".|()"
parser
regex( char *re ){
  static parser p;
  if(  !p  ){
    parser dot   = using( chr('.'), on_dot );
    parser meta  = anyof( META );
    parser escape = xthen( chr('\\'), anyof( SPECIAL "\\" ) );
    parser chr_  = using( plus( escape,  noneof( SPECIAL ) ), on_chr );
    parser expr_ = forward();
    parser atom  = PLUS( dot,
                         xthen( chr('('), thenx( expr_, chr(')') ) ),
                         chr_ );
    parser factor = into( atom, Symbol(ATOM), using( maybe( meta ), on_meta ) );
    parser term   = using( some( factor ), on_term );
    parser expr   = using( seq( term, many( xthen( chr('|'), term ) ) ), on_expr );
    *expr_ = *expr;
    p = trim( expr );
    add_global_root( p );
  }
  list r = parse( p, chars_from_string( re ) );
  return  valid( r )  ? ( x_( x_( r ) ) )  : r;
}


int test_regex(){
  parser a;
  PRINT( a = regex( "\\." ) );
  PRINT( parse( a, chars_from_string( "a" ) ) );
  PRINT( parse( a, chars_from_string( "." ) ) );
  PRINT( parse( a, chars_from_string( "\\." ) ) );
  parser b;
  PRINT( b = regex( "\\\\\\." ) );
  PRINT( parse( b, chars_from_string( "\\." ) ) );
  PRINT( take( 3, parse( b, chars_from_string( "\\." ) ) ) );
  parser r;
  PRINT( r = regex( "a?b+(c).|def" ) );
  PRINT( parse( r, chars_from_string( "abc" ) ) );
  PRINT( parse( r, chars_from_string( "abbcc" ) ) );
  PRINT( Int( garbage_collect( r ) ) );
  list s;
  PRINT( s = parse( r, chars_from_string( "def" ) ) );
  PRINT( take( 3, s ) );
  PRINT( parse( r, chars_from_string( "deff" ) ) );
  PRINT( parse( r, chars_from_string( "adef" ) ) );
  PRINT( parse( r, chars_from_string( "bcdef" ) ) );
  PRINT( Int( garbage_collect( cons( r, s ) ) ) );
  parser t;
  PRINT( t = regex( "ac|bd" ) );
  PRINT( parse( t, chars_from_string( "ac" ) ) );
  PRINT( take( 1, parse( t, chars_from_string( "bd" ) ) ) );
  PRINT( Int( garbage_collect( t ) ) );
  parser u;
  PRINT( u = regex( "ab|cd|ef" ) );
  PRINT( parse( u, chars_from_string( "ab" ) ) );
  PRINT( parse( u, chars_from_string( "cd" ) ) );
  PRINT( take( 1, parse( u, chars_from_string( "cd" ) ) ) );
  PRINT( parse( u, chars_from_string( "ef" ) ) );
  PRINT( take( 1, parse( u, chars_from_string( "ef" ) ) ) );
  PRINT( Int( garbage_collect( u ) ) );
  parser v;
  PRINT( v = regex( "ab+(c).|def" ) );
  PRINT( parse( v, chars_from_string( "def" ) ) );
  PRINT( take( 2, parse( v, chars_from_string( "def" ) ) ) );
  parser w;
  PRINT( w = regex( "a?b|c" ) );
  PRINT( parse( w, chars_from_string( "a" ) ) );
  PRINT( parse( w, chars_from_string( "b" ) ) );
  PRINT( take( 3, parse( w, chars_from_string( "c" ) ) ) );
  PRINT( Int( garbage_collect( w ) ) );
  return 0;
}


int test_env(){
  object e = env( 0, 2, Symbol(F), Int(2), Symbol(X), Int(4) );
  PRINT( e );
  PRINT( assoc( Symbol(F), e ) );
  PRINT( assoc( Symbol(X), e ) );
  return 0;
}

object b( void *v, object o ){
  //PRINT( o );
  return  one( cons( Int( - x_( o )->Int.i ), xs_( o ) ) );
}

int test_parsers(){
  list ch = chars_from_string( "a b c 1 2 3 d e f 4 5 6" );
  {
    parser p = result( Int(42) );
    PRINT( parse( p, ch ) );
    PRINT( Int( garbage_collect( ch ) ) );
  }
  {
    parser q = zero();
    PRINT( parse( q, ch ) );
    PRINT( Int( garbage_collect( ch ) ) );
  }
  {
    parser r = item();
    PRINT( r );
    PRINT( parse( r, ch ) );
    PRINT( x_( parse( r, ch ) ) );
    PRINT( take( 1, x_( parse( r, ch ) ) ) );
    PRINT( x_( take( 1, x_( parse( r, ch ) ) ) ) );
    PRINT( take( 1, x_( take( 1, x_( parse( r, ch ) ) ) ) ) );
    PRINT( parse( bind( r, Operator( 0, b ) ), ch ) );
    PRINT( Int( garbage_collect( cons( ch, r ) ) ) );
  }
  {
    parser s = plus( item(), alpha() );
    PRINT( s );
    PRINT( parse( s, ch ) );
    PRINT( take( 2, parse( s, ch ) ) );
    PRINT( Int( garbage_collect( ch ) ) );
  }
  {
    parser t = lit( Int( 'a' ) );
    PRINT( parse( t, ch ) );
    parser u = str( "a b c" );
    PRINT( parse( u, ch ) );
    PRINT( Int( garbage_collect( cons( ch, cons( t, u ) ) ) ) );
  }
  return 0;
}

int par_main(){
  return 
	  obj_main(),
	  test_env(), test_parsers(),
	  test_regex();
}
