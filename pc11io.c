#include <stdarg.h>
#include <string.h>
#include "pc11io.h"

static parser pprintf_grammar( list args );
static parser pscanf_grammar( list args );
static parser nonzero( void );

static fBinOperator sum;

static fOperator  print_literal;
static fOperator  print_char;
static fOperator  print_string;

static fPredicate is_nonzero;

static fOperator  summarize;
static fOperator  always_zero;
static fOperator  convert_char;
static fOperator  convert_string;

static fOperator  on_space;
static fOperator  on_percent;
static fOperator  on_char;
static fOperator  on_string;
static fOperator  on_literal;
static fOperator  on_terms;

int pprintf( char const *fmt, ... ){
  if(  ! fmt  ) return  0;
  static va_list v;
  va_start( v, fmt );
    static parser p;
    if(  ! p  )
      p = pprintf_grammar( env( NIL_, 1,
				Symbol(ARGS), Void( &v ) ) );
    object r = parse( p, chars_from_str( (char*)fmt ) );
  va_end( v );
  if(  not_ok( r )  ) return  0;
  return  first( rest( r ) )->Int.i;
}

int pscanf( char const *fmt, ... ){
  if(  ! fmt  ) return  0;
  static va_list v;
  va_start( v, fmt );
    static parser p;
    if(  ! p  )
      p = pscanf_grammar( env( NIL_, 1,
			       Symbol(ARGS), Void( &v ) ) );
    list fp = parse( p, chars_from_str( (char *)fmt ) );
    parser f = first( rest( fp ) );
    list r = parse( f, chars_from_file( stdin ) );
  va_end( v );
  return  valid( r )  ? first( rest( r ) )->Int.i
                      : 0;
}

static parser
pprintf_grammar( list args ){
  parser directive = ANY( bind( chr('%'), Operator( NIL_, print_literal ) ),
			  bind( chr('c'), Operator( args, print_char ) ),
			  bind( chr('s'), Operator( args, print_string ) ) );
  parser term = ANY( xthen( chr('%'), directive ),
		     bind( nonzero(), Operator( NIL_, print_literal ) ) );
  parser format = many( term );
  return  bind( format, Operator( NIL_, summarize ) );
}

static parser
pscanf_grammar( list args ){
  parser space = bind( many( anyof( " \t\n" ) ), Operator( NIL_, on_space ) );
  parser directive = ANY( bind( chr('%'), Operator( NIL_, on_percent ) ),
			  bind( chr('c'), Operator( args, on_char ) ),
			  bind( chr('s'), Operator( args, on_string ) ) );
  parser term = ANY( xthen( chr('%'), directive ),
                     bind( nonzero(), Operator( NIL_, on_literal ) ) );
  parser format = bind( many( then( space, term ) ),
			Operator( NIL_, on_terms ) );
  return  format;
}

static integer
sum( integer a, integer b ){
  return  Int( a->Int.i + b->Int.i );
}

static integer
print_literal( object env, integer it ){
  putchar( it->Int.i );
  return  Int( 1 );
}

static integer
print_char( object env, object it ){
  va_list *p = assoc_symbol( ARGS, env )->Void.pointer;
  putchar( va_arg( *p, int ) );
  return  Int( 1 );
}

static integer
print_string( object env, object it ){
  va_list *p = assoc_symbol( ARGS, env )->Void.pointer;
  char *s = va_arg( *p, char * );
  fputs( s, stdout );
  return  Int( strlen( s ) );
}

static boolean
is_nonzero( object v, list it ){
  return  Boolean( valid( it ) && it->t == INT && it->Int.i != 0 );
}

static parser
nonzero( void ){
  return  satisfy( Operator( NIL_, is_nonzero ) );
}

static integer
summarize( object v, object it ){
  return  collapse( sum, it );
}


static integer
always_zero( object v, list it ){
  return  Int( 0 );
}

static parser
pass( parser p ){
  return  bind( p, Operator( NIL_, always_zero ) );
}

static integer
convert_char( object v, list it ){
  va_list *p = assoc_symbol( ARGS, v )->Void.pointer;
  char *cp = va_arg( *p, char * );
  *cp = it->Int.i;
  return  Int( 1 );
}

static integer
convert_string( object v, list it ){
  va_list *p = assoc_symbol( ARGS, v )->Void.pointer;
  char *sp = va_arg( *p, char * );
  fill_string( &sp, it );
  return  Int( 1 );
}

static parser
on_space( object v, list it ){
  return  valid( it )  ? pass( many( anyof( " \t\n" ) ) )  : it;
}

static parser
on_percent( object v, list it ){
  return  pass( chr('%') );
}

static parser
on_char( object v, list it ){
  return  bind( item(), Operator( v, convert_char ) );
}

static parser
on_string( object v, list it ){
  return  bind( xthen( many( anyof( " \t\n" ) ), many( noneof( " \t\n" ) ) ),
		Operator( v, convert_string ) );
}

static parser
on_literal( object v, list it ){
  return  pass( literal( it ) );
}

static parser
on_terms( object v, list it ){
  return  bind( collapse( then, it ), Operator( NIL_, summarize ) );
}
