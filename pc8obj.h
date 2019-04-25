// Objects and Lists
#include <stdlib.h>

#define POINTER_TO  *
typedef union object_ POINTER_TO  object;
typedef object list;
typedef object parser;
typedef object operator;
typedef operator predicate;
typedef object boolean;
typedef object fSuspension( void * );
typedef list fParser( void *, list );
typedef object fOperator( void *, object );
typedef boolean fPredicate( void *, object );

#define NEXT10(n) n-n%10+10
typedef enum object_tag {
  INVALID, INTEGER, LIST, SUSPENSION, PARSER, OPERATOR, SYMBOL, STRING,
  NTAGS, T, SYM1 = NEXT10(T)
} tag;
union object_ { tag t;
      struct  { tag t; int i;                                } Int;
      struct  { tag t; object a, b;                          } List;
      struct  { tag t; void *v; fSuspension *f;              } Suspension;
      struct  { tag t; void *v; fParser *f;                  } Parser;
      struct  { tag t; void *v; fOperator *f;                } Operator;
      struct  { tag t; int symbol; char *pname; object data; } Symbol;
      struct  { tag t; char *string; int disposable;         } String;
      struct  { tag t; object next;                          } Header;
};
union object_ T_[1];
object new_( object o );
#define OBJECT(...) new_( (union object_[]){{ __VA_ARGS__ }} )

#define Symbol(n) OBJECT( .Symbol = { SYMBOL, n, #n } )
object Int( int i );
list one( object a );
list cons( object a, object b );
object Suspension( void *v, fSuspension *f );
parser Parser( void *v, fParser *f );
operator Operator( void *v, fOperator *f );
object String( char *s, int disposable );

void add_global_root( object a );
int garbage_collect( object local_roots );

object x_( list a );
object xs_( list a );

boolean eq( object a, object b );

list copy( list a );
list env( list tail, int n, ... );
object assoc( object a, list b );
list append( list a, list b );

list take( int n, list o );
list drop( int n, list o );

list chars_from_string( void *v );
object string_from_chars( list o );

void print( object o );
