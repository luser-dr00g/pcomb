#define PC11OBJECT_H
#include <stdlib.h>
#include <stdio.h>
#include "ppnarg.h"

typedef union object  * object;
typedef object list;
typedef object suspension;
typedef object parser;
typedef object operator;
typedef operator predicate;
typedef object boolean;
typedef object string;
typedef object symbol;
typedef object fSuspension( object env );
typedef object fParser( object env, list input );
typedef object fOperator( object env, object input );
typedef boolean fPredicate( object env, object input );
typedef object fBinOperator( object left, object right );

enum object_symbol_codes {
  T,
  END_OBJECT_SYMBOLS
};

typedef enum {
  INVALID, INT, LIST, SUSPENSION, PARSER, OPERATOR, SYMBOL, STRING, VOID
} tag;

union object { tag t;
      struct { tag t; int i;                                  } Int;
      struct { tag t; object first, rest;                     } List;
      struct { tag t; object env; fSuspension *f; char *printname; } Suspension;
      struct { tag t; object env; fParser *f; char *printname;     } Parser;
      struct { tag t; object env; fOperator *f; char *printname;   } Operator;
      struct { tag t; int code; char *printname; object data; } Symbol;
      struct { tag t; char *str; int disposable;              } String;
      struct { tag t; object next;                            } Header;
      struct { tag t; void *v;                                } Void;
};

extern object T_   /* = (union object[]){ {.t=1},{.Symbol={SYMBOL, T, "T"}}} + 1 */,
              NIL_ /* = (union object[]){ {.t=INVALID} } */;

static int
valid( object it ){
  return  it
      &&  it->t <= VOID
      &&  it->t != INVALID;
}

object     Int( int i );
boolean    Boolean( int b );
list       one( object it );
list       cons( object first, object rest );
suspension Suspension_( object env, fSuspension *f, char *printname );
#define    Suspension(env,f) Suspension_( env, f, #f )
parser     Parser_( object env, fParser *f, char *printname );
#define    Parser(env,f) Parser_( env, f, #f )
operator   Operator_( object env, fOperator *f, char *printname );
#define    Operator(env,f) Operator_( env, f, #f )
string     String( char *str, int disposable );
symbol     Symbol_( int code, char *printname, object data );
#define    Symbol(n) Symbol_( n, #n, NIL_ )
object     Void( void *ptr );

void print( object a );
void print_list( object a );

object  first( list it );
list    rest( list it );
list    take( int n, list it );
list    drop( int n, list it );
object  apply( operator op, object it );

list    chars_from_str( char *str );
list    chars_from_file( FILE *file );
list    ucs4_from_utf8( list o );
list    utf8_from_ucs4( list o );

object  collapse( fBinOperator *f, list it );
object  reduce( fBinOperator *f, int n, object *po );
#define LIST(...) \
  reduce( cons, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__, 0 } )

boolean eq( object a, object b );
boolean eq_symbol( int code, object b );

list    append( list start, list end );
list    env( list tail, int n, ... );
object  assoc( object key, list env );
object  assoc_symbol( int code, list env );
