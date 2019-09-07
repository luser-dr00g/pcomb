#define PC9OBJPRIV_H
#ifndef PC9OBJ_H
  #include "pc9obj.h"
#endif
typedef enum object_tag {
  INVALID, INTEGER, LIST, SUSPENSION, PARSER, OPERATOR, SYMBOL, STRING, VOID,
} tag;
union uobject {
                tag t;
       struct { tag t; int i;                                } Int;
       struct { tag t; object a, b;                          } List;
       struct { tag t; object v; fSuspension *f;             } Suspension;
       struct { tag t; object v; fParser *f;                 } Parser;
       struct { tag t; object v; fOperator *f;               } Operator;
       struct { tag t; int symbol; char *pname; object data; } Symbol;
       struct { tag t; char *string; int disposable;         } String;
       struct { tag t; object next;                          } Header;
       struct { tag t; void *v;                              } Void;
};
// Base Constructor
#define OBJECT(...) new_( (union uobject[]){{ __VA_ARGS__ }} )
object new_( object a );

// Force execution of Suspension
object at_( object a );

object fill_string( char **s, list o );
int obj_main( void );
