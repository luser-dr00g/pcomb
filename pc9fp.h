#define PC9FP_H
#ifndef PC9OBJ_H
  #include "pc9obj.h"
#endif

boolean eq( object a, object b );

// Association Lists
list env( list tail, int n, ... );
object assoc( object a, list b );

// Lists
list copy( list a );
list append( list a, list b );

// Functions over lists
object apply( oper f, object o );
list map( oper f, list o );
list join( list o );

// Folds
object collapse( fBinOper *f, list o );
object reduce( fBinOper *f, int n, object *po );
object rreduce( fBinOper *f, int n, object *po );

// Construct a list from variable args
#define LIST(...)  reduce( cons, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )
