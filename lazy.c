#include <stdlib.h>
#include <stdio.h>

typedef union object *object;
typedef enum tag { INT, ID, CONS, LAZY, } tag;
union object { tag t;
      struct { tag t; int    i;                } int_;
      struct { tag t; char  *s;                } id;
      struct { tag t; object a,b;              } cons;
      struct { tag t; object a,(*f)( object ); } lazy;
};
object
new_object( union object o ){
  object p = malloc( sizeof o );
  return  p ? *p = o, p : 0;
}

object
int_( int i ){
  return  new_object( (union object){ .int_ = { INT, i     } } );
}
object
id( char *s ){
  return  new_object( (union object){ .id   = { ID, s      } } );
}
object
cons( object a, object b ){
  return  new_object( (union object){ .cons = { CONS, a, b } } );
}
object
lazy( object a, object (*f)( object ) ){
  return  new_object( (union object){ .lazy = { LAZY, a, f } } );
}

object
at( object a ){
  return  a->t == LAZY ? at( a->lazy.f( a->lazy.a ) )
                       : a;
}
object
rep( object a ){
  return  cons(a, lazy( a, rep ) );
}
object
take( int n, object a ){
  return  n == 0 ? NULL
                 : (a = at( a ), cons( a->cons.a, take( n-1, a->cons.b ) ) );
}
object
add1( object a ){
  return  a ? a = at( a ),
                cons( int_( a->cons.a->int_.i + 1 ), lazy( a->cons.b, add1 ) )
            : NULL;
}

void
print( object a ){
  if(  !a  ) return;
  switch( a->t ){
  case INT:  printf("%d ", a->int_.i ); break;
  case ID:   printf("%s ", a->id.s ); break;
  case CONS: print( a->cons.a ); print( a->cons.b ); break;
  case LAZY: printf("... "); break;
  }
}

int
main(){
  print( int_(4) ), puts("");
  print( cons( id("name"), id("value") ) ), puts("");
  print( rep( int_(1) ) ), puts("");
  print( take( 5, rep( int_(1) ) ) ), puts("");
  print( take( 5, add1( rep( int_(1) ) ) ) ), puts("");
  return 0;
}
