#include "pc11csv.h"

parser csv_grammar( void );

list
csv( list input ){
  static parser csv_parser;
  if(  ! csv_parser  ) csv_parser = csv_grammar();
  object result = parse( csv_parser, input );
  return  result;
}

list
csv_file( FILE *f ){
  return  csv( chars_from_file( f ) );
}

parser
csv_grammar( void ){
  parser hex = ANY( range( 'a', 'f' ),
		    range( 'A', 'F' ),
                    digit() );
  parser number = SEQ( maybe( anyof( "+-" ) ),
		       some( digit() ),
                       maybe( chr('.') ),
                       many( digit() ) );
  parser date = SEQ( digit(),
		     maybe( digit() ),
		     chr('/'),
		     digit(),
		     maybe( digit() ),
		     chr('/'),
		     digit(),
		     maybe( digit() ) );
  parser escape = then( chr('\\'),
			either( anyof( ",\"\\/bfnrt" ),
			        SEQ( chr('u'),
				     hex,
				     maybe( hex ),
				     maybe( hex ),
				     maybe( hex ) ) ) );
  parser char_ = either( escape,
			 noneof( ",\"\r\n" ) );
  parser string_ = some( char_ );
  parser value = ANY( date,
		      number,
		      maybe( string_ ) );
  parser csv_row = then( value,
			 many( xthen( chr(','),
				      value ) ) );
  parser csv = then( csv_row,
		     many( xthen( xthen( maybe( chr('\r') ), chr('\n') ),
				  csv_row ) ) );
  return  csv;
}
