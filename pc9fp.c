#include <stdarg.h>
#include <string.h>
#include "pc9fp.h"
#include "pc9objpriv.h"

boolean
eq( object a, object b ){
  //*a = *at_( a );
  //*b = *at_( b );
  //PRINT( cons( a, b ) );
  return (
           !valid( a ) && !valid( b )  ? 1  :
           !valid( a ) || !valid( b )  ? 0  :
           a->t != b->t                ? 0  :
           a->t == SYMBOL              ? a->Symbol.symbol == b->Symbol.symbol  :
           !memcmp( a, b, sizeof *a )  ? 1  : 0
         )  ? T_  : NIL_;
}

/*
static
char *strdup( char *s ){
  char *p = calloc( strlen(s) + 1, 1 );
  return  p  ? strcpy( p, s )  : NULL;
}
*/

list
copy( list a ){
  return  !valid( a )  ? NIL_  :
          a->t == LIST  ? cons( copy( x_( a ) ), copy( xs_( a ) ) )  :
          //a->t == STRING  ? String( strdup( a->String.string ), 1 )  :
          a //new_( a )
	  ;
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
  //PRINT( r );
  return  r;
}

object
assoc( object a, list b ){
  //PRINT( cons( a, b ) );
  return  !valid( b )  ? NIL_  :
          valid( eq( a, x_( x_( b ) ) ) )  ? xs_( x_( b ) )  :
          assoc( a, xs_( b ) );
}

static list
pappend( void *v ){
  list a = assoc( Symbol(A), v );
  list b = assoc( Symbol(B), v );
  *a = *at_( a );
  return  append( a, b );
}
list
append( list a, list b ){
  //PRINT( cons( a, b ) );
  return  //!valid( b )         ? a  :
          !valid( a )         ? b  :
          a->t == SUSPENSION  ? Suspension( env( 0, 2, Symbol(A), a, Symbol(B), b ), pappend )  :
          //valid( x_( a ) )    ? 
                                cons( x_( a ), append( xs_( a ), b ) )
                                //: append( xs_( a ), b )
          ;
}


static object
papply( void *v ){
  oper f = assoc( Symbol(F), v );
  object o = assoc( Symbol(X), v );
  *o = *at_( o );
  return  valid( o )  ? f->Operator.f( f->Operator.v, o )  : NIL_;
}
object
apply( oper f, object o ){
  return  f->t == OPERATOR  ? 
              valid( o )  ?
                  o->t == SUSPENSION  ? Suspension( env( 0, 2, Symbol(F), f, Symbol(X), o ), papply )
                        : f->Operator.f( f->Operator.v, o )
                  : f->Operator.f( f->Operator.v, o )    // for using( maybe(), ... )
              : NIL_;
  return  f->t == OPERATOR  ? f->Operator.f( f->Operator.v, o )  : NIL_;
}


static list
pmap( void *v ){
  oper f = assoc( Symbol(F), v );
  list o = assoc( Symbol(X), v );
  *o = *at_( o );
  return  valid( o )  ? cons( apply( f, x_( o ) ), map( f, xs_( o ) ) ) : NIL_;
}
list
map( oper f, list o ){
  //PRINT( cons( f, o ) ); 
  return  valid( o )  ?
              o->t == SUSPENSION  ? Suspension( env( 0, 2, Symbol(F), f, Symbol(X), o ), pmap ) :
              cons( apply( f, x_( o ) ),
                    Suspension( env( 0, 2, Symbol(F), f, Symbol(X), xs_( o ) ), pmap ) )
                      : NIL_;
  return  valid( o )  ? cons( apply( f, x_( o ) ), map( f, xs_( o ) ) )  : NIL_;
}


static list
pjoin( void *v ){
  list o = assoc( Symbol(X), v );
  *o = *at_( o );
  return  append( x_( take( 1, o ) ), join( xs_( o ) ) );
}
list
join( list o ){
  return  valid( o )  ? 
              o->t == SUSPENSION  ? Suspension( env( 0, 1, Symbol(X), o ), pjoin )  :
                  append( x_( o ), Suspension( env( 0, 1, Symbol(X), xs_( o ) ), pjoin ) )
                      : NIL_;
  return  valid( o )  ? append( x_( o ), join( xs_( o ) ) )  : NIL_;
}

static object
do_collapse( fBinOper *f, object a, object b ){
  return  valid( b )  ? f( a, b )  : a;
}

object
collapse( fBinOper *f, list o ){
  return  valid( o )  ?
              o->t == LIST  ? do_collapse( f, collapse( f, x_( o ) ), collapse( f, xs_( o ) ) )
                            : o
                      : NIL_;
}

object
reduce( fBinOper *f, int n, object *po ){
  return  n==1  ? *po  : f( *po, reduce( f, n-1, po+1 ) );
}
