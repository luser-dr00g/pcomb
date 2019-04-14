#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pc7.h"
#include "ppnarg.h"

typedef enum    { INTEGER, LIST, SUSPENSION, PARSER, OPERATOR, SYMBOL, STRING, NTYPES } tag;
typedef object  fSuspension( void *);
typedef list    fParser( void *, list );
typedef object  fOperator( void *, object );
typedef boolean fPredicate( void *, object );
union   object_ { tag t;
        struct  { tag t; int i;                   } Int;
        struct  { tag t; object a, b;             } List;
        struct  { tag t; void *v; fSuspension *f; } Suspension;
        struct  { tag t; void *v; fParser *f;     } Parser;
        struct  { tag t; void *v; fOperator *f;   } Operator;
        struct  { tag t; int symbol; char *pname; } Symbol;
        struct  { tag t; char *string;            } String;
};
enum    symbol  { VALUE = NTYPES +10, PRED, P, Q, F, X, A, ID, USE,  ATOM, USER1 };
object new_( object o ){
  object p = calloc( 1, sizeof *p );
  return  p ?  *p = *o, p : 0;
}
#define OBJECT(...) new_( (union object_[]){{ __VA_ARGS__ }} )

object   Int( int i ){                          return  OBJECT( .Int        = { INTEGER, i       } ); }
list     one( object a ){                       return  OBJECT( .List       = { LIST, a, NULL    } ); }
list     cons( object a, object b ){            return  OBJECT( .List       = { LIST, a, b       } ); }
object   Suspension( void *v, fSuspension *f ){ return  OBJECT( .Suspension = { SUSPENSION, v, f } ); }
parser   Parser( void *v, fParser *f ){         return  OBJECT( .Parser     = { PARSER, v, f     } ); }
operator Operator( void *v, fOperator *f ){     return  OBJECT( .Operator   = { OPERATOR, v, f   } ); }
object   String( char *s ){                     return  OBJECT( .String     = { STRING, s        } ); }
#define  Symbol(x)  new_( (union object_[]){{ .Symbol = { SYMBOL, x, #x } }} )

object at( object a ){ return  a && a->t == SUSPENSION  ? at( a->Suspension.f( a->Suspension.v ) ) : a; }
object x( list a ){  return  a && a->t == LIST  ? a->List.a = at( a->List.a ) : NULL; }
object xs( list a ){ return  a && a->t == LIST  ? a->List.b = at( a->List.b ) : NULL; }

list copy( list a ){
  return  !a  ? NULL :
          a->t == LIST  ?  cons( copy( x( a ) ), copy( xs( a ) ) ) :
          new_( a );
}

boolean eq( object a, object b ){
  return  (
           !a && !b                                ? 1 :
           !a || !b                                ? 0 :
           a->t != b->t                            ? 0 :
           a->t == SYMBOL && 
             a->Symbol.symbol == b->Symbol.symbol  ? 1 :
           !memcmp( a, b, sizeof *a )              ? 1 : 0
          )  ? one(0) : NULL;
}

object assoc( object a, list b ){
  return  !b  ? NULL :
          eq( x( x( b ) ), a )  ? xs( x( b ) ) : 
          assoc( a, xs( b ) );
}

list env( list tail, int n, ... ){
  va_list v;
  va_start( v, n );
  list r = tail;
  while( n-- ){
    object a = va_arg( v, object );
    object b = va_arg( v, object );
    r = cons( cons( a, b ), r );
  }
  va_end( v );
  return r;
}

void print( object o ){
  if(  !o  ){ printf( "() " ); return; }
  switch( o->t ){
    case INTEGER:    printf( "%d ", o->Int.i );         break;
    case LIST:       printf( "(" );
                       print( o->List.a );
                       print( o->List.b );
                     printf( ") " );                    break;
    case SUSPENSION: printf( "SUSPENSION " );           break;
    case PARSER:     printf( "PARSER " );               break;
    case OPERATOR:   printf( "OPERATOR " );             break;
    case STRING:     printf( "\"%s\" ",
                             o->String.string );        break;
    case SYMBOL:     printf( "%s ", o->Symbol.pname );  break;
    default: break;
  }
}


list parse( parser p, list input ){
  return  p->t == PARSER  ? p->Parser.f( p->Parser.v, input ) : NULL;
}


list string_input( void *v ){
  char *p = v;
  return  *p  ? cons( Int( *p ), Suspension( p+1, string_input ) ) : NULL;
}


list fresult( void *v, list input ){
  return  one( cons( assoc( Symbol(VALUE), v ), input ) );
}
parser result( object v ){
  return  Parser( env( 0, 1, Symbol(VALUE), v ), fresult );
}


list fitem( void *v, list input ){
  return  input  ? one( cons( x( input ), xs( input ) ) ) : NULL;
}
parser item( void ){
  return  Parser( 0, fitem );
}


list fzero( void *v, list input ){
  return  NULL;
}
parser zero( void ){
  return  Parser( 0, fzero );
}


object apply( operator f, object o ){
  return  f->t == OPERATOR  ? f->Operator.f( f->Operator.v, o ) : NULL;
}
list map( operator f, list o ){
  return  o  ? cons( apply( f, x( o ) ), map( f, xs( o ) ) ) : NULL;
}
list last( list a ){
  return  a->List.b  ?  last( a->List.b ) : a;
}
list append( list a, list b ){
  if(  !b  ) return  a;
  if(  a  &&  a->t != LIST  ){
    printf( "append(a,b) a is not a List\n" );
    return  b;
  }
  return  a ? last( a )->List.b = b, a : b;
}
list join( list o ){
  return  o  ? append( x( o ), join( xs( o ) ) ) : NULL;
}
list fbind( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  operator f = assoc( Symbol(F), v );
  //printf( "bind v " ), print( v ), puts("");
  //printf( "bind f " ), print( f->Operator.v ), puts("");
  return  join( map(
            Operator( f->Operator.v ? append( copy( f->Operator.v ), v ) : v, 
                      f->Operator.f ),
            parse( p, input ) ) );
}
parser bind( parser p, operator f ){
  return  Parser( env( 0, 2, Symbol(P), p, Symbol(F), f ), fbind );
}


list finto( void *v, list o ){
  object id = assoc( Symbol(ID), v );
  parser q = assoc( Symbol(Q), v );
  return  parse( Parser( env( q->Parser.v, 1, id, x( o ) ), q->Parser.f ), xs( o ) );
}
parser into( parser p, object id, parser q ){
  return  bind( p, Operator( env( 0, 2, Symbol(ID), id, Symbol(Q), q ), finto ) );
}


boolean alpha( void *v, object o ){
  return  isalpha( o->Int.i )  ? one(0) : NULL;
}
boolean digit( void *v, object o ){
  return  isdigit( o->Int.i )  ? one(0) : NULL;
}
list fsat( void *v, list o ){
  predicate pred = assoc( Symbol(PRED), v );
  return  apply( pred, x( o ) )  ? one( o ) : NULL;
}
parser sat( predicate pred ){
  return  bind( item(), Operator( env( 0, 1, Symbol(PRED), pred ), fsat ) );
}
parser is_alpha( void ){
  return  sat( Operator( 0, alpha ) );
}
parser is_digit( void ){
  return  sat( Operator( 0, digit ) );
}


boolean flit( void *v, object o ){
  object x = assoc( Symbol(X), v );
  return  eq( x, o );
}
parser lit( object x ){
  return  sat( Operator( env( 0, 1, Symbol(X), x ), flit ) );
}


parser Char( int c ){
  return  lit( Int( c ) );
}


list fprepend( void *v, list o ){
  object a = assoc( Symbol(A), v );
  return  cons( cons( a, x( o ) ), xs( o ) );
}
list prepend( list a, list b ){
  return  map( Operator( env( 0, 1, Symbol(A), a ), fprepend ), b );
}
list fseq( void *v, list o ){
  parser q = assoc( Symbol(Q), v );
  return  prepend( x( o ), parse( q, xs( o ) ) );
}
parser seq( parser p, parser q ){
  //if(  !q  ) return p;
  return  bind( p, Operator( env( 0, 1, Symbol(Q), q ), fseq ) );
}


parser str( char *s ){
  return  *s  ? seq( Char( *s ), str( s+1 ) ) : result(0);
}


list fplus( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  parser q = assoc( Symbol(Q), v );
  return  append( parse( p, input ), parse( q, input ) );
}
parser plus( parser p, parser q ){
  //if(  !q  ) return p;
  return  Parser( env( 0, 2, Symbol(P), p, Symbol(Q), q ), fplus );
}


parser maybe( parser p ){
  return  plus( p, result( NULL ) );
}


parser forward( void ){
  return Parser( 0, 0 );
}
parser many( parser p ){
  parser q = forward();
  parser r = maybe( seq( p, q ) );
  *q = *r;
  return  r;
}


parser some( parser p ){
  return  seq( p, many( p ) );
}


list ftrim( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  list r = parse( p, input );
  return  r  ? one( x( r ) ) : r;
}
parser trim( parser p ){
  return  Parser( env( 0, 1, Symbol(P), p ), ftrim );
}


parser anyof( char *s ){
  return  *s  ? plus( Char( *s ), anyof( s+1 ) ) : zero();
}

list fnone( void *v, list input ){
  parser p = assoc( Symbol(P), v );
  return  parse( p, input )  ? NULL : fitem( 0, input );
}
parser noneof( char *s ){
  return  Parser( env( 0, 1, Symbol(P), anyof( s ) ), fnone );
}


list fxthen( void *v, list o ){
  return  one( cons( xs( x( o ) ), xs( o ) ) );
}
parser xthen( parser p, parser q ){
  return  bind( seq( p, q ), Operator( 0, fxthen ) );
}


list fthenx( void *v, list o ){
  return  one( cons( x( x( o ) ), xs( o ) ) );
}
parser thenx( parser p, parser q ){
  return  bind( seq( p, q ), Operator( 0, fthenx ) );
}


object do_collapse( object(*f)(object,object), object a, object b ){
  //printf( "collapse " ), print( a ), printf( ", " ), print( b ), puts("");
  return  b  ? f( a, b ) : a;
}
object collapse( object(*f)(object,object), list a ){
  return  a && a->t == LIST  ? do_collapse( f, collapse( f, x( a ) ), collapse( f, xs( a ) ) ) : a;
}



list fusing( void *v, list o ){
  operator f = assoc( Symbol(USE), v );
  f->Operator.v = v;
  return  one( cons( apply( f, x( o ) ), xs( o ) ) );
}
parser using( parser p, fOperator *f ){
  return  bind( p, Operator( env( 0, 1, Symbol(USE), Operator( 0, f ) ), fusing ) );
}



parser do_meta( parser a, object b ){
  switch( b->Int.i ){
  case '*':  return many( a );
  case '+':  return some( a );
  case '?':  return maybe( a );
  }
}

parser on_dot( void *v, object o ){ return  item(); }
parser on_chr( void *v, object o ){ return  lit( o ); }
parser on_meta( void *v, object o ){
  parser atom = assoc( Symbol(ATOM), v );
  return  o  ? do_meta( atom, o ) : atom;
}
parser on_term( void *v, object o ){ return  collapse( seq, o ); }
parser on_expr( void *v, object o ){ return  collapse( plus, o ); }
parser regex( char *re ){
  static parser p;
  if(  !p  ){
    parser  dot    = Char( '.' );
    parser  meta   = anyof( "*+?" );
    parser  chr    = noneof( "*+?.|()" );
    parser  expr_  = forward();
    parser  atom   = plus( using( dot, on_dot ),
                     plus( thenx( xthen( Char('('), expr_ ), Char(')') ),
                           using( chr, on_chr ) ) );
    //parser  factor = seq( atom, maybe( meta ) );
    parser  factor = into( atom, Symbol(ATOM), using( maybe( meta ), on_meta ) );
    parser  term   = using( seq( factor, many( factor ) ), on_term );
    parser  expr   = using( seq( term, many( xthen( Char('|'), term ) ) ), on_expr );
    *expr_ = *expr;
    //p = expr;
    p = trim( expr );
  }
  list r = parse( p, string_input( re ) );
  print( r ), puts("");
  return  r  ? trim( x( x( r ) ) ) : r;
}


list take( int n, list o ){
  return  n == 0  ? NULL  : cons( x( o ), take( n-1, xs( o ) ) );
}

list drop( int n, list o ){
  return  n == 0  ? o  : drop( n-1, xs( o ) );
}

int count_ints( list o ){
  return  !o               ? 0 :
          o->t == INTEGER  ? 1 :
          o->t == LIST     ? count_ints( o->List.a ) + count_ints( o->List.b ) :
          0;
}

object fill_string( char **s, list o){
  return  !o               ? NULL :
          o->t == INTEGER  ? *(*s)++ = o->Int.i, NULL :
          o->t == LIST     ? fill_string( s, o->List.a ), fill_string( s, o->List.b )  :
          NULL;
}

object to_string( list o ){
  char *s = calloc( count_ints( o )+1, 1 );
  object z = String( s );
  return  fill_string( &s, o ), z;
}


int accumulate_integer( int *pi, list o ){
  return  !o  ?  0 :
          o->t == INTEGER  ?  *pi = *pi * 10 + o->Int.i - '0' :
          o->t == LIST     ?  accumulate_integer( pi, o->List.a ) + accumulate_integer( pi, o->List.b ) :
          7; 
}

object to_integer( list o ){
  int z = 0;
  return  accumulate_integer( &z, o ), Int( z );
}


#define Each_Keyword(_) \
 _(int) _(char) _(float) _(double) _(struct) \
 _(auto) _(extern) _(register) _(static) \
 _(goto) _(return) _(sizeof) \
 _(break) _(continue) \
 _(if) _(else) \
 _(for) _(do) _(while) \
 _(switch) _(case) _(default) \
 _(entry)
#define Enum_name(x) k_##x ,


enum extended_symbols {
  t_int = USER1 +10, t_id,
  //k_if, k_int, k_char, k_float, k_double, k_struct, k_auto,
  Each_Keyword( Enum_name )
  USER2
};

object reduce(int n, object(*f)(object,object), object *po ){
  return  n == 1 ? *po : f( *po, reduce( n-1, f, po+1 ) );
}

#define PLUS(...)  reduce( PP_NARG(__VA_ARGS__), plus, (object []){ __VA_ARGS__ } )

object on_spaces( void *v, list o ){ return  to_string( o ); }
object on_digits( void *v, list o ){ return  cons( Symbol(t_int), to_integer( o ) ); }
#define On_keyword(x) \
  object on_##x( void *v, list o ){ return  cons( Symbol(k_##x), to_string( o ) ); }
Each_Keyword( On_keyword )
//object on_if( void *v, list o ){ return cons( Symbol(k_if), to_string( o ) ); }
//object on_int( void *v, list o ){ return cons( Symbol(k_int), to_string( o ) ); }
//object on_char( void *v, list o ){ return cons( Symbol(k_char), to_string( o ) ); }
//object on_float( void *v, list o ){ return cons( Symbol(k_float), to_string( o ) ); }
//object on_double( void *v, list o ){ return cons( Symbol(k_double), to_string( o ) ); }
//object on_struct( void *v, list o ){ return cons( Symbol(k_struct), to_string( o ) ); }
//object on_auto( void *v, list o ){ return cons( Symbol(k_auto), to_string( o ) ); }
object on_identifier( void *v, list o ){ return  cons( Symbol(t_id), to_string( o ) ); }

#define Handle_keyword(x) \
  using( str( #x ), on_##x ) ,
list token_input( void *s ){
  if(  !s  ) return NULL;
  static parser p;
  if(  !p  ){
    parser space   = using( many( anyof( " " ) ), on_spaces );
    parser digits  = using( some( is_digit() ), on_digits  );
    parser alnum = plus( is_alpha(), plus( is_digit(), anyof( "_" ) ) );
    parser keyword = PLUS( Each_Keyword( Handle_keyword ) );
//    parser keyword = plus( using( str( "if" ), on_if ),
//                     plus( using( str( "int" ), on_int ),
//                     plus( using( str( "char" ), on_char ),
//                     plus( using( str( "float" ), on_float ),
//                     plus( using( str( "double" ), on_double ),
//                     plus( using( str( "struct" ), on_struct ),
//                           using( str( "auto" ), on_auto ) ) ) ) ) ) );
    parser identifier = using( seq( plus( is_alpha(), anyof( "_" ) ), many( alnum ) ), on_identifier );
    p = seq( space, plus( digits, plus( keyword, identifier ) ) );
  }
  list r = x( parse( p, s ) );
  return  cons( x( r ), Suspension( xs( r ), token_input ) );
}

void test_tokens(){
  list tokens = token_input( string_input( "123 if blah bluh bleh" ) );
  print( take( 3, tokens ) ), puts("");
  print( drop( 3, tokens ) ), puts("");
  print( take( 2, drop( 3, tokens ) ) ), puts("");
}


void test_regex(){
  parser p = regex( ".?(b)+|a" );
  print( parse( p, string_input( "b" ) ) ), puts("");
  print( parse( p, string_input( "xbb" ) ) ), puts("");
  print( parse( p, string_input( "a" ) ) ), puts("");
}


void test_basics(){
  char *s = "my string";
  list p = string_input( s );
  list pp = string_input( "12345" );

  printf( "%c", x( p )->Int.i );
  printf( "%c", x( xs( p ) )->Int.i );
  printf( "%c", x( xs( xs( p ) ) )->Int.i );
  puts("");
  print( p ), puts("");
  puts("");

  parser q = result( Int(42) );
  print( parse( q, p ) ), puts("");
  print( parse( item(), p ) ), puts("");
  print( parse( is_alpha(), p ) ), puts("");
  print( parse( is_digit(), p ) ), puts("");
  puts("");

  parser r = seq( is_alpha(), is_alpha() );
  print( parse( r, p ) ), puts("");
  puts("");

  parser t = lit( Int('m') );
  print( parse( t, p ) ), puts("");
  puts("");

  parser u = maybe( is_alpha() );
  print( parse( u, p ) ), puts("");
  print( parse( u, pp ) ), puts("");
  puts("");

  parser v = trim( u );
  print( parse( v, p ) ), puts("");
  print( parse( v, pp ) ), puts("");
  puts("");

  parser w = many( is_alpha() );
  parser ww = trim( w );
  print( parse( w, p ) ), puts("");
  print( parse( ww, p ) ), puts("");
  puts("");

}

int pc_main(){
  //test_basics();
  //test_regex();
  test_tokens();
}

