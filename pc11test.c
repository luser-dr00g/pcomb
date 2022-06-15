#include "pc11parser.h"

enum test_symbol_codes {
  TEST = END_PARSER_SYMBOLS,
  postal_address,
  name_part,
  street_address,
  zip_part,
  END_TEST_SYMBOLS
};

static int test_basics();
static int test_parsers();
static int test_regex();
static int test_ebnf();

int main( void ){
  return  test_basics()
      ||  test_parsers()
      ||  test_regex()
      ||  test_ebnf();
}

static int
test_basics(){
  puts( __func__ );
  list ch = chars_from_str( "abcdef" );
    print( ch ), puts("");
    print_list( ch ), puts("");
  drop( 6, ch );
    print( ch ), puts("");
    print_list( ch ), puts("");
  drop( 7, ch );
    print( ch ), puts("");
    print_list( ch ), puts("");
  puts("");
  return  0;
}

static int
test_parsers(){
  puts( __func__ );
  list ch = chars_from_str( "a b c d 1 2 3 4" );
  parser p = result_is( Int(42) );
    print_list( parse( p, ch ) ), puts("");
  parser q = fails();
    print_list( parse( q, ch ) ), puts("");
  parser r = item();
    print_list( parse( r, ch ) ), puts("");
  parser s = either( alpha(), item() );
    print_list( parse( s, ch ) ), puts("");
  parser t = literal( Int('a') );
    print_list( parse( t, ch ) ), puts("");
  puts("");
  return  0;
}

static int
test_regex(){
  puts( __func__ );
  parser a = regex( "." );
  print_list( a ), puts("");
  print_list( parse( a, chars_from_str( "a" ) ) ), puts("");
  print_list( parse( a, chars_from_str( "." ) ) ), puts("");
  print_list( parse( a, chars_from_str( "\\." ) ) ), puts("");
  puts("");

  parser b = regex( "\\." );
  print_list( b ), puts("");
  print_list( parse( b, chars_from_str( "a" ) ) ), puts("");
  print_list( parse( b, chars_from_str( "." ) ) ), puts("");
  print_list( parse( b, chars_from_str( "\\." ) ) ), puts("");
  puts("");

  parser c = regex( "\\\\." );
  print_list( c ), puts("");
  print_list( parse( c, chars_from_str( "a" ) ) ), puts("");
  print_list( parse( c, chars_from_str( "." ) ) ), puts("");
  print_list( parse( c, chars_from_str( "\\." ) ) ), puts("");
  print_list( parse( c, chars_from_str( "\\a" ) ) ), puts("");
  puts("");

  parser d = regex( "\\\\\\." );
  print_list( d ), puts("");
  print_list( parse( d, chars_from_str( "a" ) ) ), puts("");
  print_list( parse( d, chars_from_str( "." ) ) ), puts("");
  print_list( parse( d, chars_from_str( "\\." ) ) ), puts("");
  print_list( parse( d, chars_from_str( "\\a" ) ) ), puts("");
  puts("");

  parser e = regex( "\\\\|a" );
  print_list( e ), puts("");
  print_list( parse( e, chars_from_str( "a" ) ) ), puts("");
  print_list( parse( e, chars_from_str( "." ) ) ), puts("");
  print_list( parse( e, chars_from_str( "\\." ) ) ), puts("");
  print_list( parse( e, chars_from_str( "\\a" ) ) ), puts("");
  puts("");
  
  parser f = regex( "[abcd]" );
  print_list( f ), puts("");
  print_list( parse( f, chars_from_str( "a" ) ) ), puts("");
  print_list( parse( f, chars_from_str( "." ) ) ), puts("");
  puts("");

  return  0;
}

static int
test_ebnf(){
  puts( __func__ );
  Symbol(postal_address);
  Symbol(name_part);
  Symbol(street_address);
  Symbol(zip_part);
  list a = ebnf(
"postal_address = name_part street_address zip_part ;\n"
"name_part = personal_part sp last_name sp opt_suffix_part EOL\n"
"          | personal_part sp name_part ;\n"
"personal_part = initial '.' | first_name ;\n"
"sp = ( ' ' | '\t' | '\n' |  ) sp | ;\n"
  );
  print_list( a ), puts("");
  //print( a ), puts("");
  object x = first( first( rest( a ) ) );
  print_list( x );
  print_list( Symbol(postal_address) );
  print_list( eq( x, Symbol(postal_address) ) ), puts("");
  puts("");

  return  0;
}
