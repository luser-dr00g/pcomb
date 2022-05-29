#include "pc11object.h"
#include <stdarg.h>
#include <string.h>

#define OBJECT(...) new_( (union object[]){{ __VA_ARGS__ }} )

object T_ = (union object[]){ {.t=1},{.Symbol={SYMBOL, T, "T"}}} + 1,
       NIL_ = (union object[]){ {.t=INVALID} };

object new_( object prototype );


object
Int( int i ){
  return  OBJECT( .Int = { INT, i } );
}

boolean
Boolean( int b ){
  return  b  ? T_  : NIL_;
}

list
one( object it ){
  return  cons( it, NIL_ );
}

list
cons( object first, object rest ){
  return  OBJECT( .List = { LIST, first, rest } );
}

suspension
Suspension( object env, fSuspension *f ){
  return  OBJECT( .Suspension = { SUSPENSION, env, f } );
}

parser
Parser( object env, fParser *f ){
  return  OBJECT( .Parser = { PARSER, env, f } );
}

operator
Operator( object env, fOperator *f ){
  return  OBJECT( .Operator = { OPERATOR, env, f } );
}

string
String( char *str, int disposable ){
  return  OBJECT( .String = { STRING, str, disposable } );
}

symbol
Symbol_( int code, char *printname, object data ){
  return  OBJECT( .Symbol = { SYMBOL, code, printname, data } );
}

object
Void( void *ptr ){
  return  OBJECT( .Void = { VOID, ptr } );
}


object
force_( object it ){
  if(  it->t != SUSPENSION  ) return  it;
  return  force_( it->Suspension.f( it->Suspension.env ) );
}


#define FORCE( function ) \
static object force_ ## function ( object it ){ \
  *it = *force_( it ); \
  return  function( it ); \
} \
object function( list it ){ \
  if(  it->t == SUSPENSION  ) return  Suspension( it, force_ ## function );

FORCE(first)
  if(  it->t != LIST  ) return  NIL_;
  return  it->List.first;
}

FORCE(rest)
  if(  it->t != LIST  ) return  NIL_;
  return  it->List.rest;
}


list
take( int n, list it ){
  if(  n == 0  ) return  NIL_;
  *it = *force_( it );
  if(  ! valid( it )  ) return  NIL_;
  return  cons( first( it ), take( n-1, rest( it ) ) );
}

list
drop( int n, list it ){
  if(  n == 0  ) return  it;
  *it = *force_( it );
  if(  ! valid( it )  ) return  NIL_;
  return  drop( n-1, rest( it ) );
}


static object
force_apply( list env ){
  operator op = first( env );
  object it = rest( env );
  *it = *force_( it );
  return  apply( op, it );
}

object
apply( operator op, object it ){
  if(  it->t == SUSPENSION  ) return  Suspension( cons( op, it ), force_apply );
  return  op->Operator.f( NIL_, it );
}


static list
force_chars_from_string( string s ){
  char *str = s->String.str;
  if(  ! *str  ) return  one( Symbol( EOF ) );
  return  cons( Int( *str ), Suspension( String( str+1, 0 ), force_chars_from_string ) );
}

list
chars_from_str( char *str ){
  if(  ! str  ) return  NIL_;
  return  Suspension( String( str, 0 ), force_chars_from_string );
}


static list
force_chars_from_file( object file ){
  FILE *f = file->Void.v;
  int c = fgetc( f );
  if(  c == EOF  ) return  one( Symbol( EOF ) );
  return  cons( Int( c ), Suspension( file, force_chars_from_file ) );
}

list
chars_from_file( FILE *file ){
  if(  ! file  ) return  NIL_;
  return  Suspension( Void( file ), force_chars_from_file );
}

static int
leading_ones( object byte ){
  if(  byte->t != INT  ) return  0;
  int x = byte->Int.i;
  return  x&0x80 ? x&0x40 ? x&0x20 ? x&0x10 ? x&8 ? x&4 ? 6
                                                    : 5
                                              : 4
                                     : 3
                            : 2
                   : 1
          : 0;
}

static int
mask_off( object byte, int m ){
  if(  byte->t != INT  ) return  0;
  int x = byte->Int.i;
  return  x & (m? (1<<(8-m))-1 :-1);
}

static list
force_ucs4_from_utf8( list input ){
  *input = *force_( input );
  object byte;
  byte = first( input ), input = rest( input );
  if(  !valid(byte)  ) return  NIL_;
  if(  eq_symbol( EOF, byte )  ) return  input;
  int lead = leading_ones( byte );
  int bits = mask_off( byte, lead );
  int n = lead;
  while(  n-- > 1  ){
    *input = *force_( input );
    byte = first( input ), input = rest( input );
    if(  eq_symbol( EOF, byte )  ) return  input;
    bits = ( bits << 6 ) | ( byte->Int.i & 0x3f );
  }
  if(  bits < ((int[]){0,0,0x80,0x800,0x10000,0x110000,0x4000000})[ lead ]  )
    fprintf( stderr, "Overlength encoding in utf8 char.\n" );
  return  cons( Int( bits ), Suspension( input, force_ucs4_from_utf8 ) );
}

list
ucs4_from_utf8( list input ){
  if(  ! input  ) return  NIL_;
  return  Suspension( input, force_ucs4_from_utf8 );
}


static list
force_utf8_from_ucs4( list input ){
  *input = *force_( input );
  object code = first( input );
  if(  eq_symbol( EOF, code )  ) return  input;
  int x = code->Int.i;
  object next = Suspension( drop( 1, input ), force_utf8_from_ucs4 );
  if(  x <= 0x7f  )
    return  cons( code, next );
  if(  x <= 0x7ff  )
    return  LIST( Int( (x >> 6)   | 0xc0 ),
                  Int( (x & 0x3f) | 0x80 ), next );
  if(  x <= 0xffff )
    return  LIST( Int(   (x >> 12)         | 0xe0 ),
                  Int( ( (x >> 6) & 0x3f ) | 0x80 ),
	          Int( (  x       & 0x3f ) | 0x80 ), next );
  if(  x <= 0x10ffff  )
    return  LIST( Int(   (x >> 18)          | 0xf0 ),
	          Int( ( (x >> 12) & 0x3f ) | 0x80 ),
                  Int( ( (x >> 6)  & 0x3f ) | 0x80 ),
	          Int( (  x        & 0x3f ) | 0x80 ), next );
  if(  x <= 0x3ffffff  )
    return  LIST( Int(   (x >> 24)          | 0xf8 ),
	          Int( ( (x >> 18) & 0x3f ) | 0x80 ),
	          Int( ( (x >> 12) & 0x3f ) | 0x80 ),
                  Int( ( (x >> 6)  & 0x3f ) | 0x80 ),
	          Int( (  x        & 0x3f ) | 0x80 ), next );
  if(  x <= 0x3fffffff  )
    return  LIST( Int(   (x >> 30)          | 0xfc ),
	          Int( ( (x >> 24) & 0x3f ) | 0x80 ),
	          Int( ( (x >> 18) & 0x3f ) | 0x80 ),
	          Int( ( (x >> 12) & 0x3f ) | 0x80 ),
                  Int( ( (x >> 6)  & 0x3f ) | 0x80 ),
	          Int( (  x        & 0x3f ) | 0x80 ), next );
  fprintf( stderr, "Invalid unicode code point in ucs4 char.\n" );
  return  next;
}

list
utf8_from_ucs4( list input ){
  if(  ! input  ) return  NIL_;
  return  Suspension( input, force_utf8_from_ucs4 );
}


object
collapse( fBinOperator *f, list it ){
  if(  !valid( it )  ) return  it;
  return  f( first( it ), collapse( f, rest( it ) ) );
}

object
reduce( fBinOperator *f, int n, object *po ){
  return  n==1  ? *po  : f( *po, reduce( f, n-1, po+1 ) );
}


boolean
eq( object a, object b ){
  return  Boolean(
            !valid( a ) && !valid( b )  ? 1  :
            !valid( a ) || !valid( b )  ? 0  :
            a->t != b->t                ? 0  :
            a->t == SYMBOL              ? a->Symbol.code == b->Symbol.code  :
            !memcmp( a, b, sizeof *a )  ? 1  : 0
          );
}

boolean
eq_symbol( int code, object b ){
  return  eq( (union object[]){ {.Symbol = {SYMBOL, code, "", 0} } }, b );
}


list
env( list tail, int n, ... ){
  va_list v;
  va_start( v, n );
  list r = tail;
  while( n-- ){
    object a = va_arg( v, object );
    object b = va_arg( v, object );
    r = cons( cons( a, b ), r );
  }
  va_end( v );
  return  r;
}

object
assoc( object key, list b ){
  if(  !valid( b )  ) return  NIL_;
  object pair = first( b );
  if(  valid( eq( key, first( pair ) ) )  )
    return  rest( pair );
  else
    return  assoc( key, rest( b ) );
}

object
assoc_symbol( int code, list b ){
  return  assoc( (union object[]){ {.Symbol = {SYMBOL, code, "", 0}} }, b );
}



static list allocation_list = NULL;

object
new_( object prototype ){
  object record = calloc( 2, sizeof *record );
  if(  record  ){
    record[0] = (union object){ .Header = { 0, allocation_list } };
    allocation_list = record;
    record[1] = *prototype;
  }
  return  record + 1;
}
