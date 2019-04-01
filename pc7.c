#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

typedef union object *object;
typedef enum { INT, LIST, SUSP, PARSER, OPER } tag;
union object { tag t;
      struct { tag t; int i; } Int;
      struct { tag t; object a, b; } List;
      struct { tag t; void *v; object (*f)( void * ); } Susp;
      struct { tag t; void *v; object (*f)( void *, object ); } Parser;
      struct { tag t; void *v; object (*f)( void *, object ); } Oper;
};

object new_( union object o ){
  object p = calloc( 1, sizeof o );
  return  p ? *p = o, p : 0;
}
#define OBJECT(...) new_( (union object){ __VA_ARGS__ } )

object Int( int i ){               return  OBJECT( .Int    = { INT, i        } ); }
object one( object a ){            return  OBJECT( .List   = { LIST, a, NULL } ); }
object cons( object a, object b ){ return  OBJECT( .List   = { LIST, a, b    } ); }

object Susp( void *v, object (*f)( void * ) ){           return  OBJECT( .Susp   = { SUSP, v, f    } ); }
object Parser( void *v, object (*f)( void *, object ) ){ return  OBJECT( .Parser = { PARSER, v, f  } ); }
object Oper( void *v, object (*f)( void *, object ) ){   return  OBJECT( .Oper   = { OPER, v, f    } ); }

object at( object a ){ return  a && a->t == SUSP  ? at( a->Susp.f( a->Susp.v ) ) : a; }
object x( object a ){  return  a && a->t == LIST  ? a->List.a : NULL; }
object xs( object a ){ return  a && a->t == LIST  ? a->List.b = at( a->List.b ) : NULL; }

object parse( object p, object input ){
  return  p->t == PARSER  ? p->Parser.f( p->Parser.v, input ) : NULL;
}


object string_input( void *v ){
  char *p = v;
  return  *p  ? cons( Int( *p ), Susp( p+1, string_input ) ) : NULL;
}


object fresult( void *v, object input ){
  return  one( cons( v, input ) );
}
object result( object v ){
  return  Parser( v, fresult );
}


object fitem( void *v, object input ){
  return  input  ? one( cons( x( input ), xs( input ) ) ) : NULL;
}
object item( void ){
  return  Parser( 0, fitem );
}


object apply( object f, object o ){
  return  f->t == OPER  ? f->Oper.f( f->Oper.v, o ) : NULL;
}
object map( object f, object o ){
  return  o  ? cons( apply( f, x( o ) ), map( f, xs( o ) ) ) : NULL;
}
object last( object a ){
  return  a->List.b  ?  last( a->List.b ) : a;
}
object append( object a, object b ){
  return  a ? last( a )->List.b = b, a : b;
}
object join( object o ){
  return  o  ? append( x( o ), join( xs( o ) ) ) : NULL;
}
object fbind( void *v, object input ){
  object o = v;
  object p = x( o );
  object f = xs( o );
  return  join( map( f, parse( p, input ) ) );
}
object bind( object p, object f ){
  return  Parser( cons( p, f ), fbind );
}


int alpha( object o ){
  return isalpha( o->Int.i );
}
int digit( object o ){
  return isdigit( o->Int.i );
}
object fsat( void *v, object o ){
  int (*pred)( object ) = v;
  return  pred( x( o ) )  ? one( o ) : NULL;
}
object sat( int (*pred)( object ) ){
  return bind( item(), Oper( pred, fsat ) );
}
object is_alpha( void ){
  return  sat( alpha );
}
object is_digit( void ){
  return  sat( digit );
}


object seq( object p, object q ){
}

void print( object o ){
  if(  !o  ){ printf( "() " ); return; }
  switch( o->t ){
    case INT: printf( "%d ", o->Int.i ); break;
    case LIST: printf( "(" ); print( o->List.a ); print( o->List.b ); printf( ") " ); break;
    case SUSP: printf( "SUSP " ); break;
    default: break;
  }
}

int main(){
  char *s = "my string";
  object p = string_input( s );
  printf( "%c", x( p )->Int.i );
  printf( "%c", x( xs( p ) )->Int.i );
  printf( "%c", x( xs( xs( p ) ) )->Int.i );
  puts("");
  print( p ), puts("");
  object q = result( Int(42) );
  print( parse( q, p ) ), puts("");
  print( parse( item(), p ) ), puts("");
  print( parse( is_alpha(), p ) ), puts("");
  print( parse( is_digit(), p ) ), puts("");
}
