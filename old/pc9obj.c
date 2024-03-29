#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "pc9objpriv.h"

static void mark_objects( list a );
static int sweep_objects( list *po );

// The T_ object has a dummy allocation record
boolean T_ = &(1[(union uobject[]){{ .t = 1 },{ .Symbol = { SYMBOL, T, "T" } }}]),
        NIL_ = (union uobject[]){{ .t = INVALID }};

static list global_roots = NULL;
static list allocation_list = NULL;

// Allocate an allocation record and uobject, return pointer to uobject
object
new_( object a ){
  object p = calloc( 2, sizeof *p );
  return  p  ? p[0] = (union uobject){ .Header = { 0, allocation_list } },
               allocation_list = p,
               p[1] = *a,
               &p[1]
             : 0;
}



// Constructors

object
Int( int i ){
  return  OBJECT( .Int = { INTEGER, i } );
}

list
one( object a ){
  return  cons( a, NIL_ );
}

list
cons( object a, object b ){
  return  OBJECT( .List = { LIST, a, b } );
}

object
Suspension( object env, fSuspension *f ){
  return  OBJECT( .Suspension = { SUSPENSION, env, f } );
}

parser
Parser( object env, fParser *f ){
  return  OBJECT( .Parser = { PARSER, env, f } );
}

oper
Operator( object env, fOperator *f ){
  return  OBJECT( .Operator = { OPERATOR, env, f } );
}

object
String( char *s, int disposable ){
  return  OBJECT( .String = { STRING, s, disposable } );
}

object
Symbol_( int symbol, char *printname, object data ){
  return  OBJECT( .Symbol = { SYMBOL, symbol, printname, data } );
}

object
GenSymbol( char *prefix ){
  static int next_symbol = -1;
  int n = strlen(prefix) + 4 + 1;
  char *printname = malloc( n );
  if(  printname  ) snprintf( printname, n, "%s" "%04d", prefix, - next_symbol );
  return  Symbol_( next_symbol--, printname, 0 );
}

object
Void( void *v ){
  return  OBJECT( .Void = { VOID, v } );
}

object
clone( object o ){
  return  new_( o );
}


// Garbage Collector

object
add_global_root( object a ){
  global_roots = cons( a, global_roots );
  return  a;
}

int
garbage_collect( object local_roots ){
  mark_objects( local_roots );
  mark_objects( global_roots );
  return  sweep_objects( &allocation_list );
}

static tag *
mark( object a ){
  return  &a[-1].Header.t;
}

static void
mark_objects( list a ){
  if(  !valid(a) || *mark( a )  ) return;
  *mark( a ) = 1;
  switch(  a->t  ){
  case LIST:       mark_objects( a->List.a ); 
                   mark_objects( a->List.b );       break;
  case PARSER:     mark_objects( a->Parser.env );     break;
  case OPERATOR:   mark_objects( a->Operator.env );   break;
  case SYMBOL:     mark_objects( a->Symbol.data );  break;
  case SUSPENSION: mark_objects( a->Suspension.env ); break;
  }
}

// This one's a little weird. The argument is a pointer to
// a pointer to the left side of the most recent
// (allocation record, object data) pair. The allocation
// record is accessible through the .Header member.
// (*po+1) points to the object data.
static int
sweep_objects( list *po ){
  int count = 0;
  while(  *po  )
    if(  *mark( *po+1 )  ){
      *mark( *po+1 ) = 0;
      po = &(*po)->Header.next;
    } else {
      object z = *po;
      *po = (*po)->Header.next;
      if(  (z+1)->t == STRING && (z+1)->String.disposable  )
        free( (z+1)->String.string );
      free( z );
      ++count;
    }
  return  count;
}



// Force execution of Suspension
object
force_( object a ){
  return  !valid( a ) || a->t != SUSPENSION  ?  a
          : force_( a->Suspension.f( a->Suspension.env ) );
}


// Lists

static object
force_x_( object v ){
  list a = v;
  *a = *force_( a );
  return  x_( a );
}
object
x_( list a ){
  return  valid( a )  ?
              a->t == LIST        ? a->List.a             :
              a->t == SUSPENSION  ? Suspension( a, force_x_ )  : NIL_
          : NIL_;
}

static object
force_xs_( object v ){
  list a = v;
  *a = *force_( a );
  return  xs_( a );
}
object
xs_( list a ){
  return  valid( a )  ?
              a->t == LIST        ? a->List.b              :
              a->t == SUSPENSION  ? Suspension( a, force_xs_ )  : NIL_
          : NIL_;
}

list
take( int n, list o ){
  if(  n == 0  ) return NIL_;
  if(  !o  ) return NIL_;
  *o = *force_( o );
  return  valid( o )  ? cons( x_( o ), take( n-1, xs_( o ) ) )  : NIL_;
}

list
drop( int n, list o ){
  if(  n == 0  ) return o;
  if(  !o  ) return NIL_;
  *o = *force_( o );
  return  valid( o )  ? drop( n-1, xs_( o ) )  : NIL_;
}


static list
force_chars_from_string( object s ){
  char *p = s->String.string;
  return  *p  ?
            cons( Int( *p ),
		  Suspension( String( p+1, 0 ), force_chars_from_string ) )
          : cons( Symbol(EOF), 0 );
}
list
chars_from_string( char *p ){
  return  p  ?  Suspension( String( p, 0 ), force_chars_from_string )  : NIL_;
}


static list
force_chars_from_file( object file ){
  FILE *f = file->Void.v;
  int c = fgetc( f );
  return  c != EOF  ?
            cons( Int( c ), Suspension( file, force_chars_from_file ) )
          : cons( Symbol(EOF), 0 );
}
list
chars_from_file( FILE *f ){
  return  f  ? Suspension( Void( f ), force_chars_from_file )  : NIL_;
}


// 00000000 00-7f 01111111 0   7f
// 10000000 80-bf 10111111 1   3f
// 11000000 c0-df 11011111 2   1f
// 11100000 e0-ef 11101111 3   0f
// 11110000 f0-f7 11110111 4   07
// 11111000 f8-fb 11111011 5   03
// 11111100 fc-ff 11111111 6   03
static int
leading_ones( object x ){
  unsigned int y = x->Int.i;
  return  y <= 0x7f  ? 0 :
          y <= 0xbf  ? 1 :
          y <= 0xdf  ? 2 :
          y <= 0xef  ? 3 :
          y <= 0xf7  ? 4 :
          y <= 0xfb  ? 5 :
          y <= 0xff  ? 6 : -1;
}

static object
mask_off( object x, int m ){
  return  Int( x->Int.i & 
               ((int[]){ -1, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x03 })[ m + 1 ] );
}

static object
remove_pos( object x ){
  return  x->t == LIST  ? x_( x )  : x;
}

static object
process_utf8_byte( object x, list v ){
  return Int( ( x->Int.i << 6 ) |
	      ( remove_pos( x_( take( 1, v ) ) )->Int.i & 0x3f ) );
}

static list force_ucs4_from_utf8( list v );
static object
work_around_pos( list (*f)(list), list x, list v ){
  object c = x_( x );
  object pos = xs_( x );
  object r = f( cons( c, xs_( v ) ) );
  object y = x_( r );
  if(  y->t == SYMBOL && y->Symbol.symbol == EOF  ) return y;
  //*y = *cons( y, pos );
  //return  r;
  return  cons( cons( y, pos ), xs_( r ) );
}

static list
force_ucs4_from_utf8( list v ){
  if(  !valid( v )  ) return  NIL_;
  object x = x_( take( 1, v ) );
  if(  x && x->t == LIST ){
    return  work_around_pos( force_ucs4_from_utf8, x, v );
  }
  if(  x && x->t == SYMBOL && x->Symbol.symbol == EOF  ) return  v;
  int lead = 0;
  if(  valid( x )  ){
    x = mask_off( x, lead = leading_ones( x ) );
    switch(  lead  ){
    case 6:  x = process_utf8_byte( x, v = drop( 1, v ) ); //fallthrough
    case 5:  x = process_utf8_byte( x, v = drop( 1, v ) ); //fallthrough
    case 4:  x = process_utf8_byte( x, v = drop( 1, v ) ); //fallthrough
    case 3:  x = process_utf8_byte( x, v = drop( 1, v ) ); //fallthrough
    case 2:  x = process_utf8_byte( x, v = drop( 1, v ) ); //fallthrough
    case 1:
    default: v = drop(1, v); break;
    }
  }

  if (x && x->Int.i < ((int[]){0,0,0x80,0x800,0x10000})[lead])
    fprintf( stderr, "overlength encoding in utf8 char\n" );
  return  x && x->Int.i != EOF ?
            cons( x, Suspension( v, force_ucs4_from_utf8 ) )
          : Symbol(EOF);
}
list
ucs4_from_utf8( list o ){
  return  valid( o )  ? Suspension( o, force_ucs4_from_utf8 ) : NIL_;
}

// x=format .=data
//              data bits 
//      maxval
//               7                                                     x... ....
//          7f                                                         0111 1111
//              11                                           xxx. .... xx.. ....
//        7 ff                                               1101 1111 1011 1111
//              16                                 xxxx .... xx.. .... xx.. ....
//       ff ff                                     1110 1111 1011 1111 1011 1111
//              21                       xxxx x... xx.. .... xx.. .... xx.. ....
//    10 ff ff                           1111 0111 1011 1111 1011 1111 1011 1111 
//              26             xxxx xx.. xx.. .... xx.. .... xx.. .... xx.. ....
//  3 ff ff ff                 1111 1011 1011 1111 1011 1111 1011 1111 1011 1111
//              32   xxxx xx.. xx.. .... xx.. .... xx.. .... xx.. .... xx.. ....
// 3f ff ff ff       1111 1101 1011 1111 1011 1111 1011 1111 1011 1111 1011 1111

static list force_utf8_from_ucs4( list v );
list utf_one_byte( object x, list v ){
  return  x->Int.i != EOF  ? 
	    cons( x, Suspension( drop( 1, v ), force_utf8_from_ucs4 ) )
          : Symbol(EOF);
}
static list utf_two_byte( object x, list v ){
  return  cons( Int( (x->Int.i >> 6)    | 0xc0 ),
	  cons( Int( (x->Int.i & 0x3f ) | 0x80 ),
		Suspension( drop( 1, v ), force_utf8_from_ucs4 ) ) );
}
static list utf_three_byte( object x, list v ){
  return  cons( Int(   (x->Int.i >> 12)          | 0xe0 ),
	  cons( Int( ( (x->Int.i >>  6) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >>  0) & 0x3f ) | 0x80 ),
		Suspension( drop( 1, v ), force_utf8_from_ucs4 ) ) ) );
}
static list utf_four_byte( object x, list v ){
  return  cons( Int(   (x->Int.i >> 18)           | 0xf0 ),
	  cons( Int( ( (x->Int.i >> 12) & 0x3f )  | 0x80 ),
	  cons( Int( ( (x->Int.i >>  6) & 0x3f )  | 0x80 ),
	  cons( Int( ( (x->Int.i >>  0) & 0x3f )  | 0x80 ),
		Suspension( drop( 1, v ), force_utf8_from_ucs4 ) ) ) ) );
}
static list utf_five_byte( object x, list v ){
  return  cons( Int(   (x->Int.i >> 24)          | 0xf8 ),
	  cons( Int( ( (x->Int.i >> 18) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >> 12) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >>  6) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >>  0) & 0x3f ) | 0x80 ),
		Suspension( drop( 1, v ), force_utf8_from_ucs4 ) ) ) ) ) );
}
static list utf_six_byte( object x, list v ){
  return  cons( Int(   (x->Int.i >> 30)          | 0xfc ),
	  cons( Int( ( (x->Int.i >> 24) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >> 18) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >> 12) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >>  6) & 0x3f ) | 0x80 ),
	  cons( Int( ( (x->Int.i >>  0) & 0x3f ) | 0x80 ),
		Suspension( drop( 1, v ), force_utf8_from_ucs4 ) ) ) ) ) ) );
}

static list
force_utf8_from_ucs4( list v ){
  if(  !valid( v )  ) return  NIL_;
  object x = x_( take( 1, v ) );
  if(  !valid( x )  ) return  NIL_;
  if(  x->t == LIST  ){
    return  work_around_pos( force_utf8_from_ucs4, x, v );
  }
  if(  x->t == SYMBOL && x->Symbol.symbol == EOF  ) return  v;
  if(       x->Int.i <=       0x7f  ) return  utf_one_byte(   x, v );
  else if(  x->Int.i <=      0x7ff  ) return  utf_two_byte(   x, v );
  else if(  x->Int.i <=     0xffff  ) return  utf_three_byte( x, v );
  else if(  x->Int.i <=   0x10ffff  ) return  utf_four_byte(  x, v );
  else if(  x->Int.i <=  0x3ffffff  ) return  utf_five_byte(  x, v );
  else if(  x->Int.i <= 0x3fffffff  ) return  utf_six_byte(   x, v );
  else {
    fprintf(stderr, "Invalid unicode code point in ucs4 char.\n");
    return  drop( 1, v );
  }
}

list
utf8_from_ucs4( list v ){
  return  valid( v )  ?  Suspension( v, force_utf8_from_ucs4 ) : NIL_;
}


static int
count_ints( list o ){
  return  !o               ? 0 :
          o->t == SUSPENSION ? *o = *force_( o ), count_ints( o ) :
          o->t == INTEGER  ? 1 :
          o->t == LIST     ? count_ints( o->List.a ) + count_ints( o->List.b ) :
          0;
}

object
fill_string( char **s, list o ){
  return  !o    ? NULL :
          o->t == INTEGER  ? *(*s)++ = o->Int.i, NULL :
          o->t == LIST     ? fill_string( s, o->List.a ),
                             fill_string( s, o->List.b ) :
          NULL;
}

object
string_from_chars( list o ){
  char *s = calloc( count_ints( o ) + 1, 1 );
  object z = String( s, 1 );
  return  fill_string( &s, o ), z;
}

void
print( object a ){
  if(  !a  ){ printf( "() " ); return; }
  switch( a->t ){
  case INTEGER:    printf( "%d ", a->Int.i );            break;
  case LIST:       printf( "(" ),
                     print( a->List.a ), printf( ". " ), print( a->List.b ),
                   printf( ") " );                       break;
  case SUSPENSION: printf( "... " );                     break;
  case PARSER:     printf( "Parser " );                  break;
  case OPERATOR:   printf( "Oper " );                    break;
  case STRING:     printf( "\"%s\"", a->String.string ); break;
  case SYMBOL:     printf( "%s ", a->Symbol.pname );     break;
  case VOID:       printf( "VOID" );                     break;
  case INVALID:    printf( "_ " );                       break;
  default:         printf( "INVALID " );                 break;
  }
}

static void
print_listn( list a ){
  switch(  a  ? a->t  : 0  ){
  default:   print( a ); return;
  case LIST: print_list( x_( a ) ),
             print_listn( xs_( a ) ); return;
  }
}

void
print_list( list a ){
  switch(  a  ? a->t  : 0  ){
  default:   print( a ); return;
  case LIST: printf( "(" ),
               print_list( x_( a ) ),
               print_listn( xs_( a ) ),
             printf( ")" ); return;
  }
}

static void
print_alln( list a ){
  if(  a  ) drop( 1, a );
  switch(  a  ? a->t  : 0  ){
  default:  print( a ); return;
  case LIST: print_all( x_( a ) ),
             print_alln( xs_( a ) ); return;
  }
}

void
print_all( list a ){
  if(  a  ) drop( 1, a );
  switch(  a  ? a->t  : 0  ){
  default:   print( a ); return;
  case LIST: printf( "(" ),
               print_all( x_( a ) ),
               print_alln( xs_( a ) ),
             printf( ")" ); return;
  }
}

void
print_flat( list a ){
  if(  !a  ) return;
  if(  a->t != LIST  ){ print( a ); return; }
  print_flat( a->List.a );
  print_flat( a->List.b );
}

void
print_data( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  case LIST:   print_data( a->List.a),
               print_data( a->List.b );  break;
  case STRING: printf( "%s", a->String.string );  break;
  case SYMBOL: print_data( a->Symbol.data );  break;
  default: print( a );
  }
}

static void
print_escaped_string( char *s ){
  printf( "\"" );
  for(  ; *s; s++  )
    if(  ( isgraph((int)*s) && !strchr("\"\'\\",*s) ) || *s==' '  ) printf( "%c", *s );
    else printf( "\\%c", *s=='\n'? 'n' :
                         *s=='\t'? 't' : *s );
  printf( "\" " );
}

void
print_dot( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  case LIST:   printf( "(" ),
                 print_dot( a->List.a ), printf( ". " ), print_dot( a->List.b ),
               printf( ") " );  break;
  case STRING: print_escaped_string( a->String.string ); break;
  case SYMBOL: printf("<%s:", a->Symbol.pname );
               print_dot( a->Symbol.data ); 
               printf("> ");  break;
  default: print( a );
  }
}

static void
indent( int level ){
  printf( "\n" );
  while( level-- ) printf( "  " );
}

static void
print_path( list path ){
  if(  !path  ) return;
  print_path( xs_(path) );
  object part = x_(path);
  if(  part->t == LIST  ) part = x_(part);
  printf( "%s", part->String.string );
}

static void
update_path( list path ){
  for(  ; path; path = xs_(path)  ){
    object part = x_( path );
    if(  part->t == LIST  ){
      *part = *xs_( part );
    } else if(  !strcmp( part->String.string, "|- " ) ||
                !strcmp( part->String.string, "<> " )  ){
      path->List.a = String("   ", 0);
    } else if(  !strcmp( part->String.string, "-- " )  ){
      path->List.a = String("|  ", 0);
    }
  }
}

static void
digest( list path ){
  printf("\n");
  print_path( path );
  printf("   ");
  update_path( path );
}

static void
print_tree_branch( list path, list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  case LIST:   print_tree_branch(
                   cons( cons( String("-- ", 0), String( a->List.b  ? "|  "  : "   ", 0 ) ),
                         path ),
                   a->List.a ),
	       print_tree_branch( 
                   cons( cons( String("|- ", 0), String("   ", 0 ) ),
                         path ),
                   a->List.b );  break;
  case STRING: digest( path ),
               print_escaped_string( a->String.string ); break;
  case SYMBOL: digest( path ),
               printf("<%s:", a->Symbol.pname ),
               print_tree_branch( cons( String("<> ", 0), path ), a->Symbol.data ),
               printf( "> " );  break;
  default: print( a );
  }
}

void
print_tree( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  case LIST:   print_tree_branch( cons( String("-- ", 0), 0 ), a->List.a ),
		 print_tree_branch( cons( String("|- ", 0), 0), a->List.b );  break;
  case SYMBOL: printf("<%s:", a->Symbol.pname ),
                 print_tree_branch( cons( String("<> ", 0), 0 ), a->Symbol.data ),
               printf( "> " );  break;
  default: print( a );
  }
}


static int
test_basics(){
  list ch = chars_from_string( "abcdef" );
  PRINT( ch );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 1, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( x_( ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( x_( xs_( ch ) ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 1, x_( xs_( ch ) ) ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 5, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( ch );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 6, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 1, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 2, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 2, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 2, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  PRINT( take( 2, ch ) );
  PRINT( Int( garbage_collect( ch ) ) );
  return  0;
}

int obj_main(){
  return  test_basics();
}
