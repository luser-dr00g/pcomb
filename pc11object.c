#define _BSD_SOURCE
#include "pc11object.h"
#include <stdarg.h>
#include <string.h>

#define OBJECT(...) new_( (union object[]){{ __VA_ARGS__ }} )

object T_ = (union object[]){ {.t=1}, {.Symbol={SYMBOL, T, "T"}} } + 1,
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
Suspension_( object env, fSuspension *f, const char *printname ){
  return  OBJECT( .Suspension = { SUSPENSION, env, f, printname } );
}

parser
Parser_( object env, fParser *f, const char *printname ){
  return  OBJECT( .Parser = { PARSER, env, f, printname } );
}

operator
Operator_( object env, fOperator *f, const char *printname ){
  return  OBJECT( .Operator = { OPERATOR, env, f, printname } );
}

string
String( char *str, int disposable ){
  return  OBJECT( .String = { STRING, str, disposable } );
}

symbol
Symbol_( int code, const char *printname, object data ){
  return  OBJECT( .Symbol = { SYMBOL, code, printname, data } );
}

object
Void( void *pointer ){
  return  OBJECT( .Void = { VOID, pointer } );
}


int
length( list ls ){
  return  valid( ls )  ?  valid( first( ls ) ) + length( rest( ls ) ) : 0;
}

void
fill_string( char **str, list ls ){
  if(  valid( ls )  ){
    *(*str)++ = first( ls )->Int.i;
    fill_string( str, rest( ls ) );
  }
}


string
to_string( list ls ){
  char *str = calloc( 1 + length( ls ), 1 );
  string s = OBJECT( .String = { STRING, str, 1 } );
  fill_string( &str, ls );
  return  s;
}


void
print( object a ){
  switch(  a  ? a->t  : 0  ){
  default: printf( "() " ); break;
  case INT: printf( "'%c' ", a->Int.i ); break;
  //case INT: printf( "%d ", a->Int.i ); break;
  case LIST: printf( "(" ), print( a->List.first ), printf( "." ),
                            print( a->List.rest ), printf( ")" ); break;
  case SUSPENSION: printf( "...(%s) ", a->Suspension.printname ); break;
  case PARSER: printf( "Parser(%s", a->Parser.printname ),
               printf( ", " ), print( a->Parser.env ),
               printf( ") " ); break;
  case OPERATOR: printf( "Oper(%s", a->Operator.printname ),
                 printf( ", " ), print( a->Operator.env ),
                 printf( ") " ); break;
  case STRING: printf( "\"%s\" ", a->String.str ); break;
  case SYMBOL: printf( "%s ", a->Symbol.printname ); break;
  //case SYMBOL: printf( "%d:%s ", a->Symbol.code, a->Symbol.printname ); break;
  case VOID: printf( "VOID " ); break;
  }
}


static void
print_listn( object a ){
  if(  ! valid( a )  ) return;
  switch(  a->t  ){
  default: print( a ); break;
  case LIST: print_list( first( a ) ),
             print_listn( rest( a ) ); break;
  }
}

void
print_list( object a ){
  switch(  a  ? a->t  : 0  ){
  default: print( a ); break;
  case LIST: printf( "(" ), print_list( first( a ) ),
                            print_listn( rest( a ) ), printf( ") " ); break;
  }
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
  return  op->Operator.f( op->Operator.env, it );
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
  FILE *f = file->Void.pointer;
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
  int ones = leading_ones( byte );
  int bits = mask_off( byte, ones );
  int n = ones;
  while(  n-- > 1  ){
    *input = *force_( input );
    byte = first( input ), input = rest( input );
    if(  eq_symbol( EOF, byte )  ) return  input;
    bits = ( bits << 6 ) | ( byte->Int.i & 0x3f );
  }
  if(  bits < ((int[]){0,0,0x80,0x800,0x10000,0x110000,0x4000000})[ ones ]  )
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
  //puts( "in collapse" );
  //print_list( it ), puts("");
  if(  !valid( it )  ) return  it;
  object right = collapse( f, rest( it ) );
  if(  !valid( right )  ) return  first( it );
  return  f( first( it ), right );
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
append( list start, list end ){
  if(  ! valid( start )  ) return  end;
  return  cons( first( start ), append( rest( start ), end ) );
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

static int next_symbol_code = -2;

symbol
symbol_from_string( string s ){
  list ls = allocation_list;
  while(  ls != NULL && valid( ls + 1 )  ){
    //print( ls + 1 );
    if(  ls[1].t == SYMBOL
    &&  strcmp( ls[1].Symbol.printname, s->String.str ) == 0  ){
      return  ls + 1;
    }
    ls = ls[0].Header.next;
  }
  return  Symbol_( next_symbol_code--, strdup( s->String.str ), NIL_ );
}
