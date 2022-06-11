#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include "pc9par.h"
#include "pc9objpriv.h"

static list parse_result( object v, list input );
static list parse_zero( object v, list input );
static list parse_item( object v, list input );
static list parse_bind( object v, list input );
static list parse_plus( object v, list input );
static list force_r_plus( object v );
static list force_q_plus( object v );
static list parse_sat( object v, list input );
static boolean pred_alpha( object v, object o );
static boolean pred_digit( object v, object o );
static list parse_noneof( object v, list input );
static boolean pred_lit( object v, object o );
static list each_prepend( object v, list o );
static list prepend( list a, list b );
static list oper_seq( object v, list output );
static list oper_xthen( object v, list o );
static list oper_thenx( object v, list o );
static list oper_into( object v, list o );
static list parse_trim( object v, list input );
static list parse_using( object v, list o );

list
parse( parser p, list input ){
  return  valid( p ) && p->t == PARSER && valid( input ) ?
            p->Parser.f( p->Parser.env, input )
          : NIL_;
}


// Unit constructors

parser
result( object a ){
  return  Parser( env( 0, 1, Symbol(VALUE), a ), parse_result );
  return  Parser( a, parse_result );
}

static list
parse_result( object v, list input ){
  return  one( cons( assoc_symbol( VALUE, v ), input ) );
  return  one( cons( v, input ) );
}


parser
zero( void ){
  return  Parser( 0, parse_zero );
}

static list
parse_zero( object v, list input ){
  return  NIL_;
}


parser
item( void ){
  return  Parser( 0, parse_item );
}

static list
parse_item( object v, list input ){
  drop( 1, input );
  //return  valid( input )  ? one( cons( x_( input ), xs_( input ) ) ) : NIL_;
  object i = x_( input );
  if(  i && i->t == LIST  ) i = x_( i );
  return  valid( input )  ? one( cons( i, xs_( input ) ) ) : NIL_;
  //return  valid( input ) ? one( cons( x_( take( 1, input ) ), xs_( input ) ) )  : NIL_;  //strict
  //return  valid( input ) ? one( cons( x_( input ), xs_( input ) ) )  : NIL_;             //lazy
}


// Attach operator to process parser results

parser
bind( parser p, oper f ){
  return  Parser( env( 0, 2, Symbol(BIND_P), p, Symbol(BIND_F), f ), parse_bind );
}

static list
parse_bind( object v, list input ){
  parser p = assoc_symbol( BIND_P, v );
  oper f = assoc_symbol( BIND_F, v );
  list r = parse( p, input );
  return  valid( r )  ?
            join( map(
	      Operator( valid( f->Operator.env )  ? append( copy( f->Operator.env ), v )  : v,
	        f->Operator.f ),
             r ) )
	  : NIL_;
}


// Logical OR

parser
plus( parser p, parser q ){
  if(  !q  ) return  p;
  return  Parser( env( 0,2,Symbol(PLUS_P),p,Symbol(PLUS_Q),q ), parse_plus );
}

static list
parse_plus( object v, list input ){
  parser p = assoc_symbol( PLUS_P, v );
  parser q = assoc_symbol( PLUS_Q, v );
  list r = parse( p, input );
  object qq = Suspension( env(0,2,Symbol(PLUS_Q),q,Symbol(PLUS_X),input), force_q_plus );
  return  valid( r )  ? 
            r->t == SUSPENSION  ?
              Suspension( env(0,2,Symbol(PLUS_R),r,Symbol(PLUS_Q),qq), force_r_plus )
	    : append( r, qq )
	  : qq;
}

static list
force_r_plus( object v ){
  list r = assoc_symbol( PLUS_R, v );
  object qq = assoc_symbol( PLUS_Q, v );
  *r = *force_( r );
  return  valid( r )  ?
            append( r, qq )
          : qq;
}

static list
force_q_plus( object v ){
  parser q = assoc_symbol( PLUS_Q, v );
  list input = assoc_symbol( PLUS_X, v );
  return  parse( q, input );
}


// check item with predicate

parser
sat( predicate pred ){
  return  bind( item(), Operator( env( 0,1,Symbol(SAT_PRED),pred ), parse_sat ) );
}

static list
parse_sat( object v, list input ){
  predicate pred = assoc_symbol( SAT_PRED, v );
  object r = apply( pred, x_( input ) );
  return  valid( r )  ?
            one( cons( x_( input ), xs_( input ) ) )
          : NIL_;
}


// characters

parser
alpha( void ){
  return  sat( Operator( 0, pred_alpha ) );
}

static boolean
pred_alpha( object v, object o ){
  return  isalpha( o->Int.i )  ? T_  : NIL_;
}


parser
digit( void ){
  return  sat( Operator( 0, pred_digit ) );
}

static boolean
pred_digit( object v, object o ){
  return  isdigit( o->Int.i )  ? T_  : NIL_;
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


parser
noneof( char *s ){
  return  Parser( env( 0, 1, Symbol(NONEOF_P), anyof( s ) ), parse_noneof );
}

static list
parse_noneof( object v, list input ){
  parser p = assoc_symbol( NONEOF_P, v );
  object r = parse( p, input );
  *r = *force_( r );
  return  valid( r )  ? NIL_  : parse_item( 0, input );
}


// check item against typed object a

parser
lit( object a ){
  return  sat( Operator( env( 0, 1, Symbol(LIT_X), a ), pred_lit ) );
}

static boolean
pred_lit( object v, object o ){
  object a = assoc_symbol( LIT_X, v );
  return  eq( a, o );
}


// Sequencing

parser
seq( parser p, parser q ){
  if(  !q  ) return  p;
  return  bind( p, Operator( env( 0, 1, Symbol(SEQ_Q), q ), oper_seq ) );
}

static list
oper_seq( object v, list output ){
  parser q = assoc_symbol( SEQ_Q, v );
  return  prepend( x_( output ), parse( q, xs_( output ) ) );
}

static list
prepend( list a, list b ){
  return  map( Operator( env( 0, 1, Symbol(PREPEND_A), a ), each_prepend ), b );
}

static list
each_prepend( object v, list o ){
  object a = assoc_symbol( PREPEND_A, v );
  return  valid( a )  ? cons( cons( a, x_( o ) ), xs_( o ) )  : o;
}


parser
xthen( parser p, parser q ){
  return  bind( seq( p, q ), Operator( 0, oper_xthen ) );
}

static list
oper_xthen( object v, list o ){
  return  one( cons( xs_( x_( o ) ), xs_( o ) ) );
}


parser
thenx( parser p, parser q ){
  return  bind( seq( p, q ), Operator( 0, oper_thenx ) );
}

static list
oper_thenx( object v, list o ){
  return  one( cons( x_( x_( o ) ), xs_( o ) ) );
}


parser
into( parser p, object id, parser q ){
  return  bind( p,
		Operator( env( 0,2,Symbol(INTO_ID),id,Symbol(INTO_Q),q ), oper_into ) );
}

static list
oper_into( object v, list o ){
  object id = assoc_symbol( INTO_ID, v );
  parser q = assoc_symbol( INTO_Q, v );
  return  parse( Parser( env(q->Parser.env,1,id,x_(o)), q->Parser.f ), xs_( o ) );
}


// Construct emtpy parser to fill in later

parser
forward( void ){
  return  Parser( 0, 0 );
}


// ? * +

parser
maybe( parser p ){
  return  plus( p, result(0) );
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


// return only first result

parser
trim( parser p ){
  return  Parser( env( 0, 1, Symbol(TRIM_P), p ), parse_trim );
  //return  Parser( p, parse_trim );
}

static list
parse_trim( object v, list input ){
  parser p = assoc_symbol( TRIM_P, v );
  //parser p = v;
  list r = parse( p, input );
  return  valid( r )  ? one( x_( take( 1, r ) ) )  : r;
}


// map results through user callback

parser
using( parser p, object v, fOperator *f ){
  return  bind( p,
		Operator( env( 0,1,Symbol(USING_F),Operator(v,f) ), parse_using ) );
}

static list
parse_using( object v, list o ){
  oper f = assoc_symbol( USING_F, v );
  return  one( cons( apply( f, x_( o ) ), xs_( o ) ) );
}


// Construct a parser defined by regular expression

static parser
apply_metachar( parser a, object o ){
  switch( o->Int.i ){
  case '*':  return  many( a ); break;
  case '+':  return  some( a ); break;
  case '?':  return  maybe( a ); break;
  } return  a;
}
static parser
on_metachar( object v, object o ){
  parser atom = assoc_symbol( ATOM, v );
  return  valid( o ) ? apply_metachar( atom, o )  : atom;
}
static parser  on_dot( object v, object o ){ return  item(); }
static parser  on_chr( object v, object o ){ return  lit( o ); }
static parser  on_term( object v, object o ){ return  collapse( seq, o ); }
static parser  on_expr( object v, object o ){ return  collapse( plus, o ); }

#define META     "*+?"
#define SPECIAL  META ".|()"
parser
regex( char *re ){
  static parser p;
  if(  !p  ){
    parser dot    = using( chr('.'), 0, on_dot );
    parser meta   = anyof( META );
    parser escape = xthen( chr('\\'), anyof( SPECIAL "\\" ) );
    parser chr_   = using( plus( escape,  noneof( SPECIAL ) ),
			   0, on_chr );
    parser expr_  = forward();
    parser atom   = PLUS( dot,
                          xthen( chr('('), thenx( expr_, chr(')') ) ),
                          chr_ );
    parser factor = into( atom, Symbol(ATOM),
			  using( maybe( meta ), 0, on_metachar ) );
    parser term   = using( some( factor ),
			   0, on_term );
    parser expr   = using( seq( term, many( xthen( chr('|'), term ) ) ),
			   0, on_expr );
    *expr_ = *expr;
    //p = trim( expr );
    p = expr;
    add_global_root( p );
  }
  list r = parse( p, chars_from_string( re ) );
  drop( 5 , r );
  PRINT( r );
  return  valid( r )  ? ( x_( x_( r ) ) )  : r;
}


// example, simple printf() implementatino

object sum( object a, object b ){ return  Int( a->Int.i + b->Int.i ); }

boolean nz( object v, object o ){ return  o->Int.i ? T_ : NIL_; }


static object p_char( object v, list o ){
  va_list *p = v->Void.v; return  putchar(va_arg( *p, int )), Int(1);
}
static object p_string( object v, list o ){
  va_list *p = v->Void.v;
  char *s = va_arg( *p, char* );
  return  fputs( s, stdout ), Int(strlen( s ));
}
static object p_lit( object v, list o ){
  return  putchar( o->Int.i ), Int(1);
}

static object on_fmt( object v, list o ){ return  collapse( sum, o ); }

int
pprintf( char const *fmt, ... ){
  if(  !fmt  ) return  0;
  static va_list v;
  va_start( v, fmt );
  static parser p;
  if(  !p  ){
    parser directive = PLUS( using( chr('%'), 0, p_lit ),
                             using( chr('c'), Void( &v ), p_char ),
                             using( chr('s'), Void( &v ), p_string ) );
    parser term = PLUS( xthen( chr('%'), directive ),
                        using( sat( Operator( 0, nz ) ), 0, p_lit ) );
    parser format = many( term );
    p = using( format, 0, on_fmt );
    add_global_root( p );
  }
  object r = parse( p, chars_from_string( (char*)fmt ) );
  drop( 1, r );
  va_end( v );
  return  x_( x_( r ) )->Int.i;
}


// example, simple scanf() implementation

static object  convert_char( object v, list o ){
  va_list *p = v->Void.v;
  char *cp = va_arg( *p, char* );
  *cp = o->Int.i;
  return  Int(1);
}
static object  convert_string( object v, list o ){
  va_list *p = v->Void.v;
  char *sp = va_arg( *p, char* );
  fill_string( &sp, o );
  return  Int(1);
}

static parser  on_char( object v, list o ){
  return  using( item(), v, convert_char );
}
static parser  on_string( object v, list o ){
  return  using( xthen( many( anyof( " \t\n" ) ),  many( noneof( " \t\n" ) ) ),
		  v, convert_string );
}

static object  r_zero( object v, list o ){ return  Int(0); }
static parser  pass( parser p ){ return  using( p, 0, r_zero ); }

static parser  on_space( object v, list o ){ return  valid( o )  ? pass( many( anyof( " \t\n" ) ) )  : o; }
static parser  on_percent( object v, list o ){ return  pass( chr('%') ); }
static parser  on_lit( object v, list o ){ return  pass( lit( o ) ); }

static object  sum_up( object v, list o ){ return  collapse( sum, o ); }

static parser  on_terms( object v, list o ){ return  using( collapse( seq, o ), 0, sum_up ); }

int
pscanf( char const *fmt, ... ){
  if(  !fmt  ) return  0;
  static va_list v;
  va_start( v, fmt );
  static parser p;
  if(  !p  ){
    parser space = using( many( anyof( " \t\n" ) ), 0, on_space );
    parser directive = PLUS( using( chr('%'), 0, on_percent ),
                             using( chr('c'), Void( &v ), on_char ),
                             using( chr('s'), Void( &v ), on_string ) );
    parser term  = PLUS( xthen( chr('%'), directive ),
                         using( sat( Operator( 0, nz ) ), 0, on_lit ) );
    parser format = many( seq( space, term ) );
    p = using( format, 0, on_terms );
    add_global_root( p );
  }
  list fp = parse( p, chars_from_string( (char*)fmt ) );
  drop( 1, fp );
  parser f = x_( x_( fp ) );
  if(  !valid( f )  ) return 0;
  list r = parse( f, chars_from_file( stdin ) );
  drop( 1, r );
  va_end( v );
  return  valid( r ) ? x_( x_( r ) )->Int.i : 0;
}


int test_pscanf(){
  char c;
  PRINT( Int( pscanf( "" ) ) );
  PRINT( Int( pscanf( "abc" ) ) );
  PRINT( Int( pscanf( "  %c", &c ) ) );
  PRINT( string_from_chars( Int( c ) ) );
  char buf[100];
  PRINT( Int( pscanf( "%s", buf ) ) );
  PRINT( String( buf, 0 ) );
  return 0;
}

int test_pprintf(){
  PRINT( Int( pprintf( "%% abc %c %s\n", 'x', "123" ) ) );
  return  0;
}


int test_regex(){
  parser a;
  PRINT( a = regex( "\\." ) );
  PRINT( parse( a, chars_from_string( "a" ) ) );
  PRINT( parse( a, chars_from_string( "." ) ) );
  PRINT( parse( a, chars_from_string( "\\." ) ) );
  PRINT( Int( garbage_collect( 0 ) ) );
  PRINT( a = regex( "." ) );
  PRINT( parse( a, chars_from_string( "abc" ) ) );
  PRINT( Int( garbage_collect( 0 ) ) );
  parser b;
  PRINT( b = regex( "\\\\\\." ) );
  PRINT( parse( b, chars_from_string( "\\." ) ) );
  PRINT( take( 3, parse( b, chars_from_string( "\\." ) ) ) );
  PRINT( Int( garbage_collect( 0 ) ) );
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
  PRINT( assoc_symbol( F, e ) );
  PRINT( assoc_symbol( X, e ) );
  return 0;
}

object b( object v, object o ){
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
    PRINT(                           parse( r, ch )         );
    PRINT(                       x_( parse( r, ch ) )       );
    PRINT(              take( 1, x_( parse( r, ch ) ) )     );
    PRINT(          x_( take( 1, x_( parse( r, ch ) ) ) )   );
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
	  //obj_main(),
	  //test_env(), 
          test_parsers(),
	  test_regex(),
          //test_pprintf(),
          //test_pscanf(),
          0;
}
