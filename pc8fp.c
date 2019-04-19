#include "pc8fp.h"

object
apply( operator f, object o ){
  return  f->t == OPERATOR  ? f->Operator.f( f->Operator.v, o ) : NULL;
}

list
map( operator f, list o ){
  return  o  ? cons( apply( f, x_( o ) ), map( f, xs_( o ) ) ) : NULL;
}

list
join( list o ){
  return  o  ? append( x_( o ), join( xs_( o ) ) ) : NULL;
}

static object
do_collapse( object(*f)(object,object), object a, object b ){
  return  b  ? f( a, b ) : a;
}

object
collapse( object(*f)(object,object), list a ){
  return  a && a->t == LIST  ? do_collapse( f, collapse( f, x_( a ) ), collapse( f, xs_( a ) ) ) : a;
}

object
reduce( int n, object(*f)(object,object), object *po ){
  return  n == 1 ? *po : f( *po, reduce( n-1, f, po+1 ) );
}

