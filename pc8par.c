#include <ctype.h>
#include "pc8par.h"


list
parse( parser p, list input ){
  return  p->t == PARSER  ? p->Parser.f( p->Parser.v, input ) : NULL;
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
  return  NULL;
}
parser
zero( void ){
  return  Parser( 0, pzero );
}

static list
pitem( void *v, list input ){
  return  input && input->t != INVALID  ? one( cons( x_( input ), xs_( input ) ) ) : NULL;
}
parser
item( void ){
  return  Parser( 0, pitem );
}


static list
pbind( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  operator f = assoc( Symbol(F), v );
  return  join( map(
            Operator( f->Operator.v ? append( copy( f->Operator.v ), v ) : v,
                      f->Operator.f ),
            parse( p, input ) ) );
}
parser
bind( parser p, operator f ){
  return  Parser( env( 0, 2, Symbol(P), p, Symbol(F), f ), pbind );
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
  object qq = Suspension( env( 0, 2, Symbol(Q), q, Symbol(X), input ), cplus );
  return  r  ? append( r, qq ) : qq;
//  return  append( parse( p, input ), Suspension( env( 0, 2, Symbol(Q), q, Symbol(X), input ), cplus ) );
}
static list
v1_pplus( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  parser q = assoc( Symbol(Q), v );
  return  append( parse( p, input ), parse( q, input ) );
}
parser
plus( parser p, parser q ){
  if(  !q  ) return  p;
  return  Parser( env( 0, 2, Symbol(P), p, Symbol(Q), q ), pplus );
}


static list
psat( void *v, list input ){
  predicate pred = assoc( Symbol(PRED), v );
  return  apply( pred, x_( input ) )  ? one( input ) : NULL;
}
parser
sat( predicate pred ){
  return  bind( item(), Operator( env( 0, 1, Symbol(PRED), pred ), psat ) );
}

static boolean
palpha( void *v, object o ){
  return  isalpha( o->Int.i )  ? T_ : NULL;
}
parser
alpha( void ){
  return  sat( Operator( 0, palpha ) );
}

static boolean
pdigit( void *v, object o ){
  return  isdigit( o->Int.i )  ? T_ : NULL;
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
  return  *s  ? seq( chr( *s ), str( s+1 ) ) : result(0);
}

parser
anyof( char *s ){
  return  *s  ? plus( chr( *s ), anyof( s+1 ) ) : zero();
}

static list
pnone( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  return  parse( p, input )  ? NULL : pitem( 0, input );
}
parser
noneof( char *s ){
  return  Parser( env( 0, 1, Symbol(P), anyof( s ) ), pnone );
}


static list
pprepend( void *v, list o ){
  object a = assoc( Symbol(A), v );
  return  cons( cons( a, x_( o ) ), xs_( o ) );
}
static list
prepend( list a, list b ){
  return  map( Operator( env( 0, 1, Symbol(A), a ), pprepend ), b );
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

parser forward( void ){
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
  parser p = assoc( Symbol(P), v );
  list r = parse( p, input );
  return  r  ? one( x_( r ) ) : r;
}
parser
trim( parser p ){
  return  Parser( env( 0, 1, Symbol(P), p ), ptrim );
}


static list
pusing( void *v, list o ){
  operator f = assoc( Symbol(USE), v );
  f->Operator.v = v;
  return  one( cons( apply( f, x_( o ) ), xs_( o ) ) );
}
parser
using( parser p, fOperator *f ){
  return  bind( p, Operator( env( 0, 1, Symbol(USE), Operator( 0, f ) ), pusing ) );
}


static parser  do_meta( parser a, object o ){
  switch( o->Int.i ){
  case '*':  return  many( a );
  case '+':  return  some( a );
  case '?':  return  maybe( a );
  }
}
static parser  on_meta( void *v, object o ){
  parser atom = assoc( Symbol(ATOM), v );
  return  o  ? do_meta( atom, o ) : atom;
}
static parser  on_dot( void *v, object o ){ return  item(); }
static parser  on_chr( void *v, object o ){ return  lit( o ); }
static parser  on_term( void *v, object o ){ return  collapse( seq, o ); }
static parser  on_expr( void *v, object o ){ return  collapse( plus, o ); }

parser
regex( char *re ){
  static parser p;
  if(  !p  ){
    parser dot    = using( chr( '.' ), on_dot );
    parser meta   = anyof( "*+?" );
    parser chr_   = using( noneof( "*+?.|()" ), on_chr );
    parser expr_  = forward();
    parser atom   = PLUS( dot,
                          thenx( xthen( chr('('), expr_ ), chr(')') ),
                          chr_ );
    parser factor = into( atom, Symbol(ATOM), using( maybe( meta ), on_meta ) );
    parser term   = using( seq( factor, many( factor ) ), on_term );
    parser expr   = using( seq( term, many( xthen( chr('|'), term ) ) ), on_expr );
    *expr_ = *expr;
    add_global_root( p = trim( expr ) );
  }
  list r = parse( p, chars_from_string( re ) );
  return  r  ? trim( x_( x_( r ) ) ) : r;
}

#include <stdio.h>

int test_basics(){
  char *s = "my string";
  list p = chars_from_string( s );
  list pp = chars_from_string( "12345" );

  printf( "%c", x_( p )->Int.i );
  printf( "%c", x_( xs_( p ) )->Int.i );
  printf( "%c", x_( xs_( xs_( p ) ) )->Int.i );
  puts("");
  print( p ), puts("");
  puts("");

  parser q = result( Int(42) );
  print( parse( q, p ) ), puts("");
  print( parse( item(), p ) ), puts("");
  print( parse( alpha(), p ) ), puts("");
  print( parse( digit(), p ) ), puts("");
  puts("");

  parser r = seq( alpha(), alpha() );
  print( parse( r, p ) ), puts("");
  puts("");

  parser t = lit( Int('m') );
  print( parse( t, p ) ), puts("");
  puts("");

  parser u = maybe( alpha() );
  print( parse( u, p ) ), puts("");
  print( parse( u, pp ) ), puts("");
  puts("");

  parser v = trim( u );
  print( parse( v, p ) ), puts("");
  print( parse( v, pp ) ), puts("");
  puts("");

  parser w = many( alpha() );
  parser ww = trim( w );
  print( parse( w, p ) ), puts("");
  print( parse( ww, p ) ), puts("");
  puts("");
}

int test_regex(){
  parser p = regex( ".?(b)+|a" );
  print( parse( p, chars_from_string( "b" ) ) ), puts("");
  print( parse( p, chars_from_string( "xbb" ) ) ), puts("");
  print( parse( p, chars_from_string( "a" ) ) ), puts("");
}

//int main(){ test_basics(); test_regex(); }
