#include "c_preprocessor.h"


#define SPLIT( x, y, z )       \
            object y,          \
                   z;          \
            split( x, &y, &z )
static void split( list it, object *front, list *back ){
  *front = first( it );
  *back = rest( it );
}


static fSuspension chars_with_positions;
static list position( object item );
static parser position_grammar( void );
static fOperator new_line;


list
chars_with_positions( list input ){
  static parser position_parser;
  if(  ! position_parser  ) position_parser = position_grammar();
  object result = parse( position_parser, input );
  if(  not_ok( result )  ) return  rest( rest( result ) );
  object payload = rest( result );
  return  cons( position( first( payload ) ),
		Suspension( rest( payload ), chars_with_positions ) );
}

static list
position( object item ){
  static int row = 0,
             col = 0;
  if(  valid( eq_int( '\n', item ) )  )
    return  cons( item, cons( Int( ++ row ), Int( col = 0 ) ) );
  else
    return  cons( item, cons( Int(    row ), Int( ++ col  ) ) );
}

static parser
position_grammar( void ){
  return  either( bind( ANY( str("\r\n"),
			     chr('\r'),
			     chr('\n') ),
	                Operator( NIL_, new_line ) ),
                  item() );
}

static object
new_line( list env, object input ){
  return  Int('\n');
}


list
expand( list ts ){
  if(  ! valid( ts )  ) return  NIL_;
  SPLIT( ts, t, ts_ );

}

