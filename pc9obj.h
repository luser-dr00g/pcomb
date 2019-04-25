#define PC9OBJ_H
#include <stdlib.h>
#include <stdio.h>
#define POINTER_TO  *
typedef union uobject  POINTER_TO object;
typedef object list;
typedef object parser;
typedef object oper;
typedef oper   predicate;
typedef object boolean;
typedef object fSuspension( void * );
typedef list fParser( void *, list );
typedef object fOperator( void *, object );
typedef boolean fPredicate( void *, object );
typedef object fBinOper( object, object );

enum object_symbols {
  T, F, X, A, B,
  SYM1
};
object T_, NIL_;

int valid( object a );
object Int( int i );
list one( object a );
list cons( object a, object b );
object Suspension( void *v, fSuspension *f );
parser Parser( void *v, fParser *f );
oper Operator( void *v, fOperator *f );
object String( char *s, int disposable );
object Symbol_( int sym, char *pname );
#define Symbol(n) Symbol_( n, #n )

void add_global_root( object a );
int garbage_collect( object local_roots );

object x_( list a );
object xs_( list a );
list take( int n, list o );
list drop( int n, list o );

list chars_from_string( void *v );

void print( object o );
void print_list( list a );
#define PRINT(__) printf( "%s: %s = ", __func__, #__ ), print_list( __ ), puts("")
