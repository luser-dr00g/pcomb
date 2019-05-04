#include <stdio.h>
#include "pc9objpriv.h"

object T_ = (union uobject[]){{ .Symbol = { SYMBOL, T, "T" } }},
       NIL_ = (union uobject[]){{ .t = INVALID }};

static list global_roots = NULL;
static list allocation_list = NULL;

object
new_( object a ){
  object p = calloc( 2, sizeof *p );
  return  p  ? p[0] = (union uobject){ .Header = { 0, allocation_list } },
               allocation_list = p,
               p[1] = *a,
               &p[1]
             : 0;
}

int
valid( object a ){
  switch( a  ? a->t  : 0 ){
  default:
    return 0;
  case INTEGER:
  case LIST:
  case SUSPENSION:
  case PARSER:
  case OPERATOR:
  case SYMBOL:
  case STRING:
    return 1;
  }
}

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
Suspension( object v, fSuspension *f ){
  return  OBJECT( .Suspension = { SUSPENSION, v, f } );
}

parser
Parser( object v, fParser *f ){
  return  OBJECT( .Parser = { PARSER, v, f } );
}

oper
Operator( object v, fOperator *f ){
  return  OBJECT( .Operator = { OPERATOR, v, f } );
}

object
String( char *s, int disposable ){
  return  OBJECT( .String = { STRING, s, disposable } );
}

object
Symbol_( int sym, char *pname ){
  return  OBJECT( .Symbol = { SYMBOL, sym, pname } );
}

object
Void( void *v ){
  return  OBJECT( .Void = { VOID, v } );
}

void
add_global_root( object a ){
  global_roots = cons( a, global_roots );
}

void
mark_this( object a, int m ){
  a[-1].Header.t = m;
}

int
mark( object a ){
  return  a[-1].Header.t;
}

void
mark_objects( list a ){
  if(  !valid(a) || mark( a )  ) return;
  mark_this( a, 1 );
  switch(  a->t  ){
  case LIST:       mark_objects( a->List.a ); 
                   mark_objects( a->List.b );       break;
  case PARSER:     mark_objects( a->Parser.v );     break;
  case OPERATOR:   mark_objects( a->Operator.v );   break;
  case SYMBOL:     mark_objects( a->Symbol.data );  break;
  case SUSPENSION: mark_objects( a->Suspension.v ); break;
  }
}

int
sweep_objects( list *po ){
  int count = 0;
  while(  *po  )
    if(  (*po)->t  ){
      (*po)->t = 0;
      po = &(*po)->Header.next;
    } else {
      object z = *po;
      *po = (*po)->Header.next;
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



object
at_( object a ){
  return  valid( a ) && a->t == SUSPENSION  ? at_( a->Suspension.f( a->Suspension.v ) )  : a;
}


object
px_( object v ){
  list a = v;
  *a = *at_( a );
  return  x_( a );
}
object
x_( list a ){
  return  valid( a )  ?
              a->t == LIST        ? a->List.a             :
              a->t == SUSPENSION  ? Suspension( a, px_ )  : NIL_
          : NIL_;
}

object
pxs_( object v ){
  list a = v;
  *a = *at_( a );
  return  xs_( a );
}
object
xs_( list a ){
  return  valid( a )  ?
              a->t == LIST        ? a->List.b              :
              a->t == SUSPENSION  ? Suspension( a, pxs_ )  : NIL_
          : NIL_;
}

list
take( int n, list o ){
  if(  n == 0  ) return NIL_;
  *o = *at_( o );
  return  valid( o )  ? cons( x_( o ), take( n-1, xs_( o ) ) )  : NIL_;
}
list
drop( int n, list o ){
  if(  n == 0  ) return o;
  *o = *at_( o );
  return  valid( o )  ? drop( n-1, xs_( o ) )  : NIL_;
}


list
pchars_from_string( object v ){
  char *p = v->String.string;
  return  *p  ?  cons( Int( *p ), Suspension( String( p+1, 0 ), pchars_from_string ) )  : Symbol(EOF);
}
list
chars_from_string( char *p ){
  return  p  ?  Suspension( String( p, 0 ), pchars_from_string )  : NIL_;
}


list
pchars_from_file( object v ){
  FILE *f = v->Void.v;
  int c = fgetc( f );
  return  c != EOF  ? cons( Int( c ), Suspension( v, pchars_from_file ) )  : Symbol(EOF);
}
list
chars_from_file( FILE *f ){
  return  f  ? Suspension( Void( f ), pchars_from_file ) : NIL_;
}


static int
count_ints( list o ){
  return  !o               ? 0 :
          o->t == SUSPENSION ? *o = *at_( o ), count_ints( o ) :
          o->t == INTEGER  ? 1 :
          o->t == LIST     ? count_ints( o->List.a ) + count_ints( o->List.b ) :
          0;
}

object
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
  case SUSPENSION: printf( "... " );              break;
  case PARSER:     printf( "Parser " );                  break;
  case OPERATOR:   printf( "Oper " );                break;
  case STRING:     printf( "\"%s\"", o->String.string ); break;
  case SYMBOL:     printf( "%s ", o->Symbol.pname );     break;
  case INVALID:    printf( "_ " );                       break;
  default:         printf( "INVALID " );                 break;
  }
}

void
print_listn( list a ){
  //if(  !valid( a )  ) return;
  switch(  a  ? a->t  : 0  ){
  default: print( a ); return;
  case LIST: print_list( x_( a ) ), print_listn( xs_( a ) ); return;
  }
}

void
print_list( list a ){
  //if(  !valid( a )  ) return;
  switch(  a  ? a->t  : 0  ){
  default: print( a ); return;
  case LIST: printf( "(" ), print_list( x_( a ) ), print_listn( xs_( a ) ), printf( ")" ); return;
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
  case LIST:  print_data( a->List.a), print_data( a->List.b );  break;
  case STRING: printf( "%s", a->String.string ); break;
  case SYMBOL: print_data( a->Symbol.data );  break;
  }
}


int
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
  return 0;
}

int obj_main(){ return test_basics(); }
