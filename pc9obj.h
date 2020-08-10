#define PC9OBJ_H
#include <stdlib.h>
#include <stdio.h>
//An object is a reference to the right hand side of a 2-element array
//of tagged unions. Each object is allocated as a pair of (allocation record,
//payload) and the 'object' type is a pointer to the payload side.
#define POINTER_TO  *
typedef union uobject  POINTER_TO object;
typedef object  list;
typedef object  parser;
typedef object  oper;
typedef oper    predicate;
typedef object  boolean;
typedef object  fSuspension( object );
typedef list    fParser( object, list );
typedef object  fOperator( object, object );
typedef boolean fPredicate( object, object );
typedef object  fBinOper( object, object );

enum object_symbols {
  T, X, F, APPLY_F, APPLY_X, MAP_F, MAP_X, JOIN_X, APPEND_A, APPEND_B,
  SYM1
};
boolean T_, NIL_;

int valid( object a );  // not null and not NIL_ 

// Constructors
object  Int( int i );
list    one( object a ); // make a one element list
list    cons( object a, object b ); // list node
object  Suspension( object v, fSuspension *f );
parser  Parser( object v, fParser *f );
oper    Operator( object v, fOperator *f );
object  String( char *s, int disposable );
object  Symbol_( int sym, char *pname );
#define Symbol(n) Symbol_( n, #n )
object  GenSymbol( char *prefix );
object  Void( void *v );

// Garbage Collector
void add_global_root( object a );
int garbage_collect( object local_roots );

// Lists
object x_( list a );  // car
object xs_( list a ); // cdr
list take( int n, list o );
list drop( int n, list o );

// Lazy list producers
list chars_from_string( char *v );
list chars_from_file( FILE *v );
list ucs4_from_utf8( list o );
list utf8_from_ucs4( list o );
object string_from_chars( list o );

// Printing stuff
#define PRINT(__)      PRINT_WRAPPER( print_list, __, "= " )
void print( object o );
void print_list( list a );
void print_flat( list a );
void print_data( list a );
void print_tree( list a );
#define PRINT_FLAT(__) PRINT_WRAPPER( print_flat, __, "flat= " )
#define PRINT_DATA(__) PRINT_WRAPPER( print_data, __, "data=\n" )
#define PRINT_TREE(__) PRINT_WRAPPER( print_tree, __, "tree=\n" )
#define PRINT_WRAPPER(_, __, ___) \
  printf( "%s: %s %s", __func__, #__, ___ ), _( __ ), puts("")
