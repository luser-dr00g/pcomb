// Functional Programming functions
#include "pc8obj.h"

object apply( operator f, object o );
list map( operator f, list o );
list join( list o );
object collapse( object(*f)(object,object), list o );
object reduce( int n, object(*f)(object,object), object *po );
