#define PC10OBJECT_H
#include <stdlib.h>

typedef union object object;
typedef object list;
typedef object suspension;
typedef object parser;
typedef object boolean;
typedef object operator;
typedef operator predicate;
typedef object fSuspension( object );
typedef object fParser( object, list );
typedef object fOperator( object, object );
typedef boolean fPredicate( object, object );
typedef object fBinaryOperator( object, object );

typedef enum {
  INVALID, INT,
  LIST,   FIRST_INDEXED_TYPE = LIST,
    SUSPENSION, PARSER, OPERATOR, SYMBOL,
  STRING,  LAST_INDEXED_TYPE = STRING,
  VOID
} tag;

union object { tag t;
      struct { tag t; int i; } Int;
      struct { tag t; unsigned index; } Ref;
      struct { tag t; void *ptr; } Void;
};

struct list {
  object a, b;
};

struct suspension {
  object env;
  fSuspension *f;
};

struct parser {
  object env;
  fParser *f;
};

struct operator {
  object env;
  fOperator *f;
};

struct symbol {
  int code;
  char *printname;
  object data;
};

struct string {
  char *ptr;
  int disposable;
};

struct bank {
  size_t used, max;
  void **table;
};

struct memory {
  struct bank bank[ LAST_INDEXED_TYPE - FIRST_INDEXED_TYPE + 1 ];
};

extern
struct memory memory;

static
void *get_ptr( object x ){
  if(  x.t < FIRST_INDEXED_TYPE || x.t > LAST_INDEXED_TYPE  )
    return  NULL;
  return  memory.bank[ x.t - FIRST_INDEXED_TYPE ].table[ x.Ref.index ];
}

static
boolean T_   = { .Int = { INT, -1 } },
        NIL_ = { INVALID };

object Int( int i );
boolean Bool( int b );
list cons( object a, object b );
list one( object it );
suspension Suspension( object env, fSuspension *f );
parser Parser( object env, fParser *f );
operator Operator( object env, fOperator *f );
symbol Symbol_( int code, char *printname, object data );
#define Symbol(n) Symbol_( n, #n, NIL_ )
string String( char *ptr, int disposable );

object first( list it );
list rest( list it );
