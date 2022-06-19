#define PC11OBJECT_H
#include <stdlib.h>
#include <stdio.h>
#include "ppnarg.h"

#define IS_THE_TARGET_OF_THE_HIDDEN_POINTER_  *
typedef union object  IS_THE_TARGET_OF_THE_HIDDEN_POINTER_  object;
typedef object integer;
typedef object list;
typedef object suspension;
typedef object parser;
typedef object operator;
typedef operator binoperator;
typedef operator predicate;
typedef object symbol;
typedef object string;
typedef object boolean;
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
      struct { tag t; int i;                                             } Int;
      struct { tag t; object first, rest;                                } List;
      struct { tag t; object env; fSuspension *f; const char *printname; } Suspension;
      struct { tag t; object env; fParser *f; const char *printname;     } Parser;
      struct { tag t; object env; fOperator *f; const char *printname;   } Operator;
      struct { tag t; int code; const char *printname; object data;      } Symbol;
      struct { tag t; char *str; int disposable;                         } String;
      struct { tag t; object next; int forward;                          } Header;
      struct { tag t; void *pointer;                                     } Void;
};

extern object T_   /* = (union object[]){ {.t=1}, {.Symbol={SYMBOL, T, "T"}} } + 1 */,
              NIL_ /* = (union object[]){ {.t=INVALID} } */;

static int
valid( object it ){  // valid will also convert a boolean T_ or NIL_ to an integer 1 or 0
  return  it
      &&  it->t <= VOID
      &&  it->t != INVALID;
}

integer    Int( int i );
boolean    Boolean( int b );
list       one( object it );
list       cons( object first, object rest );
suspension Suspension_( object env, fSuspension *f, const char *printname );
#define    Suspension(env,f) Suspension_( env, f, __func__ )
parser     Parser_( object env, fParser *f, const char *printname );
#define    Parser(env,f) Parser_( env, f, __func__ )
operator   Operator_( object env, fOperator *f, const char *printname );
#define    Operator(env,f) Operator_( env, f, #f )
string     String( char *str, int disposable );
symbol     Symbol_( int code, const char *printname, object data );
#define    Symbol(n) Symbol_( n, #n, NIL_ )
object     Void( void *pointer );

int     length( list ls );
string  to_string( list ls );

void    print( object a );
void    print_list( object a );

object  first( list it );
list    rest( list it );
list    take( int n, list it );
list    drop( int n, list it );
object  apply( operator op, object it );

list    chars_from_str( char *str );
list    chars_from_file( FILE *file );
list    ucs4_from_utf8( list o );
list    utf8_from_ucs4( list o );

list    map( operator op, list it );
object  collapse( fBinOperator *f, list it );
object  reduce( fBinOperator *f, int n, object *po );
#define LIST(...) \
  reduce( cons, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )

boolean eq( object a, object b );
boolean eq_symbol( int code, object b );

list    append( list start, list end );
list    env( list tail, int n, ... );
object  assoc( object key, list env );
object  assoc_symbol( int code, list env );

symbol symbol_from_string( string s );

