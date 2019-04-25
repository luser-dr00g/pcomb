#define PC9FP_H
#ifndef PC9OBJ_H
  #include "pc9obj.h"
#endif

boolean eq( object a, object b );
list copy( list a );
list env( list tail, int n, ... );
object assoc( object a, list b );
list append( list a, list b );
object apply( oper f, object o );
list map( oper f, list o );
list join( list o );
object collapse( fBinOper *f, list o );
object reduce( fBinOper *f, int n, object *po );
