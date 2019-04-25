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
  switch( a ? a->t : 0 ){
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
Suspension( void *v, fSuspension *f ){
  return  OBJECT( .Suspension = { SUSPENSION, v, f } );
}

parser
Parser( void *v, fParser *f ){
  return  OBJECT( .Parser = { PARSER, v, f } );
}

oper
Operator( void *v, fOperator *f ){
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
  case LIST:     mark_objects( a->List.a ); 
                 mark_objects( a->List.b );      break;
  case PARSER:   mark_objects( a->Parser.v );    break;
  case OPERATOR: mark_objects( a->Operator.v );  break;
  case SYMBOL:   mark_objects( a->Symbol.data ); break;
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
  return  valid( a ) && a->t == SUSPENSION  ? at_( a->Suspension.f( a->Suspension.v ) ) : a;
}

object
px_( void *v ){
  list a = v;
  *a = *at_( a );
  return  x_( a );
}
object
x_( list a ){
  return  valid( a )  ?
              a->t == LIST        ? a->List.a            :
              a->t == SUSPENSION  ? Suspension( a, px_ ) : NIL_
          : NIL_;
}

object
pxs_( void *v ){
  list a = v;
  *a = *at_( a );
  return  xs_( a );
}
object
xs_( list a ){
  return  valid( a )  ?
              a->t == LIST        ? a->List.b             :
              a->t == SUSPENSION  ? Suspension( a, pxs_ ) : NIL_
          : NIL_;
}

list
take( int n, list o ){
  if(  n == 0  ) return NIL_;
  *o = *at_( o );
  return  cons( x_( o ), take( n-1, xs_( o ) ) );
}
list
drop( int n, list o ){
  if(  n == 0  ) return NIL_;
  *o = *at_( o );
  return  drop( n-1, xs_( o ) );
}


list
pchars_from_string( void *v ){
  char *p = v;
  return  *p  ?  cons( Int( *p ), Suspension( p+1, pchars_from_string ) ) : Symbol(EOF);
}
list
chars_from_string( void *v ){
  char *p = v;
  return  *p  ?  Suspension( p, pchars_from_string ) : Symbol(EOF);
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
  switch(  a ? a->t : 0  ){
  default: print( a ); return;
  case LIST: print_list( x_( a ) ), print_listn( xs_( a ) ); return;
  }
}

void print_list( list a ){
  //if(  !valid( a )  ) return;
  switch(  a ? a->t : 0  ){
  default: print( a ); return;
  case LIST: printf( "(" ), print_list( x_( a ) ), print_listn( xs_( a ) ), printf( ")" ); return;
  }
}


int test_basics(){
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
}

int obj_main(){ test_basics(); }
