#include "c_preprocessor.h"


#define SPLIT( x, y, z )       \
            object y,          \
                   z;          \
            split( x, &y, &z )
static void split( list it, object *front, list *back ){
  *front = first( it );
  *back = rest( it );
}


static fSuspension force_chars_with_positions;
static list position( object item, int *row, int *col );
static parser position_grammar( void );
static fOperator new_line;


list
chars_with_positions( list input ){
  return  Suspension( env( NIL_, 3,
                           Symbol(POS_ROW), Int( 0 ),
                           Symbol(POS_COL), Int( 0 ),
                           Symbol(POS_INPUT), input ),
		      force_chars_with_positions );
}
  
list
force_chars_with_positions( list ev ){
  list input = assoc_symbol( POS_INPUT, ev );
  integer row = assoc_symbol( POS_ROW, ev );
  integer col = assoc_symbol( POS_COL, ev );

  static parser position_parser;
  if(  ! position_parser  ) position_parser = position_grammar();
  object result = parse( position_parser, input );
  if(  not_ok( result )  ) return  rest( rest( result ) );

  object payload = rest( result );
  list pos = position( first( payload ), &row->Int.i, &col->Int.i );
  return  cons( pos,
		Suspension( env( NIL_, 3,
				 Symbol(POS_ROW), row,
				 Symbol(POS_COL), col,
                                 Symbol(POS_INPUT), rest( payload ) ),
			    force_chars_with_positions ) );
}

static list
position( object item, int *row, int *col ){
  if(  valid( eq_int( '\n', item ) )  )
    return  cons( item, cons( Int( ++ *row ), Int( *col = 0 ) ) );
  else
    return  cons( item, cons( Int(    *row ), Int( ++ *col  ) ) );
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

