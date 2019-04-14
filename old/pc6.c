#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef union object *object;
typedef object (*fz)( object self );
typedef object (*fo)( object self, object a );
typedef object (*foo)( object self, object a, object b );
typedef enum tag { INT, STRING, LIST, FILE_, FZ, FO, FOO } tag;
union object { tag t;
      struct { tag t; int i;              } int_;
      struct { tag t; char *s;            } string;
      struct { tag t; object a, b;        } list;
      struct { tag t; FILE *f;            } file;
      struct { tag t; object a; fz f;     } fz;
      struct { tag t; object a; fo f;     } fo;
      struct { tag t; object a; foo p;    } foo;
};
object new_object( union object o ){ object p = malloc( sizeof o ); return  p ? *p = o, p: 0; }
#define OBJECT(...) new_object( (union object){ __VA_ARGS__ } )
object int_( int i ){                        return  OBJECT( .int_   = { INT, i        } ); }
object string( char *s ){                    return  OBJECT( .string = { STRING, s     } ); }
object cons( object a, object b ){           return  OBJECT( .list   = { LIST, a, b    } ); }
object file( FILE *f ){                      return  OBJECT( .file   = { FILE_, f      } ); }
object lazy( object a, fz f ){               return  OBJECT( .fz     = { FZ, a, f      } ); }
object oper( object a, fo f ){               return  OBJECT( .fo     = { FO, a, f      } ); }
object parser( object a, foo p ){            return  OBJECT( .foo    = { FOO, a, p     } ); }

object one( object o ){ return cons( o, NULL ); }

object at( object a ){ return  a->t == FZ  ? a->fz.f( a->fz.a ) : a; }
object x( object a ){  return  a->t == LIST  ? a->list.a : NULL; }
object xs( object a ){ return  a->t == LIST  ? a->list.b = at( a->list.b ) : NULL; }  /// MUTATOR
object call( object f, object o ){  return  f->t == FO  ? f->fo.f( f->fo.a, o ) : NULL; }
object parse( object a, object b, object in ){ return  a->t == FOO  ? a->foo.p( a->foo.a, b, in ) : NULL; }


object string_input( object s ){
  char *p = s->string.s;
  return  *p  ? cons( int_( *p ), lazy( string( p+1 ), string_input ) )
	      : NULL;
}

object file_input( object f ){
  int c = fgetc( f->file.f );
  return  c != EOF  ? cons( int_( c ), lazy( f, file_input ) )
                    : NULL;
}

object filename_input( object fn ){
  FILE *f = fopen( fn->string.s, "r" );
  return  f  ? file_input( file( f ) )
             : NULL;
}

object item_is( object a, object b, object in ){
  return  in  ? one( cons( x( in ), xs( in ) ) ) : NULL;
}
object item( void ){
  return parser( 0, item_is );
}

object result_is( object a, object b, object in ){ return  one( cons( a, in ) ); }
object result( object v ){
  return  parser( v, result_is );
}


object map( object f, object o ){
  return  o && f->t==FO ? cons( f->fo.f( f->fo.a, x( o ) ), map( f, xs( o ) ) )
             : NULL;
}

object last( object a ){
  return  a->list.b ? last( a->list.b ) : a;
}

object append( object a, object b ){     /// MUTATOR
  return  last( a )->list.b = b,  a;
}

object join( object o ){
  return  append( x( o ), join( xs( o ) ) );
}

object bind_is( object a, object b, object in ){
  object p = a->list.a;
  object f = a->list.b;
  object r = parse( p, b, in );
  return  join( map( f, r ) );
}
object bind( object p, object f ){
  return  parser( cons( p, f ), bind_is );
}


object sat_is( object a, object b, object in ){
  // apply predicate to result from item()
}
object sat( fo f ){
  return  bind( item(), parser( oper( 0, f ) , sat_is ) );
}

object is_alpha( object a, object o ){
  return isalpha( o->int_.i ) ? one(0) : NULL;
}


object seq_qq( object a, object b, object in ){
  // access result from variable
}
object seq_pp( object a, object b, object in ){
  // save result in variable
}
object seq( object p, object q ){
  object qq = bind( q, seq_qq );
  object pp = bind( p, seq_pp );
}


int main(){
  object x = cons( int_(3), cons( int_(4), NULL ) );

  object p = result( int_(42) );
  printf("%d\n",   parse( p, 0, x )->list.a->list.a->int_.i);
  printf("%d\n\n", parse( p, 0, x )->list.a->list.b->list.a->int_.i);

  object r = item();
  printf("%d\n",   parse( r, 0, x )->list.a->list.a->int_.i);
  printf("%d\n\n", parse( r, 0, x )->list.a->list.b->list.a->int_.i);

  object s = seq( item(), item() );
  printf("%d\n",   parse( s, 0, x )->list.a->list.a->list.a->int_.i);
  printf("%d\n",   parse( s, 0, x )->list.a->list.a->list.b->int_.i);
  printf("%d\n\n", parse( s, 0, x )->list.a->list.b ? 1 : 0 );

  object t = sat( is_alpha );
  printf("%d\n",   parse( t, 0, x )->list.a->list.a->int_.i);
  printf("%d\n\n", parse( t, 0, x )->list.a->list.b->list.a->int_.i);
}
