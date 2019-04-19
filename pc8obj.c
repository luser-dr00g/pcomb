#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "pc8obj.h"
static list global_roots = NULL;
static list allocation_list = NULL;

object
new_( object o ){
  object p = calloc( 2, sizeof *p );
  return  p  ? p[0] = (union object_){ .Header = { 0, allocation_list } },
               allocation_list = p,
               p[1] = *o,
               &p[1]
             : 0;
}

object
Int( int i ){
  return  OBJECT( .Int = { INTEGER, i } );
}

list
one( object a ){
  return  OBJECT( .List = { LIST, a, NULL } );
}

list
cons( object a, object b ){
  //if(  (unsigned long)b == 0x6100000002  ) printf("bad b\n"),puts("");
  return  OBJECT( .List = { LIST, a, b } );
}

object
Suspension( void *v, fSuspension *f ){
  return  OBJECT( .Suspension = { SUSPENSION, v, f } );
}

parser
Parser( void *v, fParser *f ){
  return  OBJECT( .Parser = { PARSER, v, f } );
}

operator
Operator( void *v, fOperator *f ){
  return  OBJECT( .Operator = { OPERATOR, v, f } );
}

object
String( char *s, int disposable ){
  return  OBJECT( .String = { STRING, s, disposable } );
}


void
add_global_root( object a ){
  global_roots = cons( a, global_roots );
}

void mark_this( object a, int m ){
  a[-1].Header.t = m;
}

int markp( object a ){
  return  a[-1].Header.t;
}

void mark_objects( list a ){
  if(  !a  ) return;
  if(  markp( a )  ) return;
  mark_this( a, 1 );
  switch(  a->t  ){
  case LIST:  mark_objects( a->List.a ); mark_objects( a->List.b ); break;
  case PARSER:  mark_objects( a->Parser.v ); break;
  case OPERATOR:  mark_objects( a->Operator.v ); break;
  case SYMBOL:  mark_objects( a->Symbol.data ); break;
  }
}

int sweep_objects( list *po ){
  int count = 0;
  while(  *po  )
    if(  (*po)->t  ){
      (*po)->t = 0;
      po = &(*po)->Header.next;
    } else {
      object z = *po;
      *po = (*po)->Header.next;
      //printf( "sweep " ), print( z+1 ), puts("");
      if(  z[1].t == STRING && z[1].String.disposable  )
        free( z[1].String.string );
      free( z );
      ++count;
    }
  return  count;
}

int
garbage_collect( object local_roots ){
  mark_objects( local_roots );
  mark_objects( global_roots );
  return  sweep_objects( &allocation_list );
}


static object
at( object a ){
  return  a && a->t == SUSPENSION  ? at( a->Suspension.f( a->Suspension.v ) ) : a;
}

object
x_( list a ){
  return  a && a->t == LIST  ? a->List.a = at( a->List.a ) : a;
}

object
xs_( list a ){
  return  a && a->t == LIST  ? a->List.b = at( a->List.b ) : a;
}

boolean
eq( object a, object b ){
  return  (
            !a && !b                                ? 1 :
            !a || !b                                ? 0 :
            a->t != b->t                            ? 0 :
            a->t == SYMBOL &&
              a->Symbol.symbol == b->Symbol.symbol  ? 1 :
            !memcmp( a, b, sizeof *a )              ? 1 : 0
          )  ? one(0) : NULL;
}

list
copy( list a ){
  return  !a  ? NULL :
          a->t == LIST  ? cons( copy( x_( a ) ), copy( xs_( a ) ) ) :
          new_( a );
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
  return r;
}

object
assoc( object a, list b ){
  return  !b  ? NULL :
          eq( x_( x_( b ) ), a )  ? xs_( x_( b ) ) :
          assoc( a, xs_( b ) );
}

static list
last( list a ){
  return  a->List.b  ? last( a->List.b ) : a;
}

list
append( object a, object b ){
  if(  !b  ) return a;
  if(  a  &&  a->t != LIST  ){
    printf( "append(a,b) a is not a list\n" );
    return b;
  }
  return  a  ? last( a )->List.b = b, a : b;
}

list
take( int n, list o ){
  return  n == 0  ? NULL : cons( x_( o ), take( n-1, xs_( o ) ) );
}

list
drop( int n, list o ){
  return  n == 0  ? o : drop( n-1, xs_( o ) );
}

list
chars_from_string( void *v ){
  char *p = v;
  return  *p  ? cons( Int( *p ), Suspension( p+1, chars_from_string ) ) : NULL;
}

static int
count_ints( list o ){
  return  !o               ? 0 :
          o->t == INTEGER  ? 1 :
          o->t == LIST     ? count_ints( o->List.a ) + count_ints( o->List.b ) :
          0;
}

static object
fill_string( char **s, list o ){
  return  !o    ? NULL :
          o->t == INTEGER  ? *(*s)++ = o->Int.i, NULL :
          o->t == LIST     ? fill_string( s, o->List.a ), fill_string( s, o->List.b ) :
          NULL;
}

object
string_from_chars( list o ){
  char *s = calloc( count_ints( o ) + 1, 1 );
  object z = String( s, 1 );
  return  fill_string( &s, o ), z;
}

void
print( object o ){
  if(  !o  ){ printf( "() " ); return; }
  switch( o->t ){
  case INTEGER:    printf( "%d ", o->Int.i );            break;
  case LIST:       printf( "(" );
                     print( o->List.a );
                     print( o->List.b );
                   printf( ") " );                       break;
  case SUSPENSION: printf( "SUSPENSION " );              break;
  case PARSER:     printf( "PARSER " );                  break;
  case OPERATOR:   printf( "OPERATOR " );                break;
  case STRING:     printf( "\"%s\"", o->String.string ); break;
  case SYMBOL:     printf( "%s ", o->Symbol.pname );     break;
  default:         printf( "INVALID " );                 break;
  }
}
