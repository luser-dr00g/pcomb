#define PC9OBJ_H
#include <stdlib.h>
#include <stdio.h>
//An object is a reference to the right hand side of a 2-element array
//of tagged unions. Each object is allocated as a pair of (allocation record,
//payload) and the 'object' type is a pointer to the payload side.
#define POINTER_TO  *
typedef union uobject  POINTER_TO object;
typedef enum object_tag {
  INVALID, INTEGER, LIST, SUSPENSION, PARSER, OPERATOR, SYMBOL, STRING, VOID,
} tag;
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
  ZERO, T, X, F, APPLY_F, APPLY_X, MAP_F, MAP_X, JOIN_X, APPEND_A, APPEND_B,
  INPUT, POS,
  SYM1
};
extern boolean T_, NIL_;

static int valid( object a );  // not NULL and not NIL_ 

static int
valid( object a ){
  switch( a  ? *(tag*)a  : 0 ){
  default:  // null, NIL_ or unknown
    return 0;
  case INTEGER: case LIST: case SUSPENSION: case PARSER: case OPERATOR:
  case SYMBOL: case STRING: case VOID:
    return 1;
  }
}

// Constructors
object  Int( int i );
list    one( object a ); // make a one element list
list    cons( object a, object b ); // list node
object  Suspension( object env, fSuspension *f );
parser  Parser( object env, fParser *f );
oper    Operator( object env, fOperator *f );
object  String( char *s, int disposable );
object  Symbol_( int sym, char *pname, object data );
#define Symbol(n) Symbol_( n, #n, 0 )
object  GenSymbol( char *prefix );
object  Void( void *v );
object  clone( object o );

// Garbage Collector
object add_global_root( object a );
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
void print_all( list a );
void print_flat( list a );
void print_dot( list a );
void print_data( list a );
void print_tree( list a );
#define PRINT_ALL(__) PRINT_WRAPPER( print_all, __, "all= " )
#define PRINT_DOT(__) PRINT_WRAPPER( print_dot, __, "dot= " )
#define PRINT_FLAT(__) PRINT_WRAPPER( print_flat, __, "flat= " )
#define PRINT_DATA(__) PRINT_WRAPPER( print_data, __, "data=\n" )
#define PRINT_TREE(__) PRINT_WRAPPER( print_tree, __, "tree=\n" )
#define PRINT_WRAPPER(F, EXP, SPEC) \
  printf( "%s: %s %s", __func__, #EXP, SPEC ), F( EXP ), puts("")


