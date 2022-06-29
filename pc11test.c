#include <ctype.h>
#include "pc11test.h"

enum test_symbol_codes {
  TEST = END_IO_SYMBOLS,
  DIGIT,
  UPPER,
  NAME,
  NUMBER,
  EOL,
  SP,
  postal_address,
  name_part,
  street_address,
  street_name,
  zip_part,
  END_TEST_SYMBOLS
};

static int test_basics();
static int test_parsers();
static int test_regex();
static int test_ebnf();
static int test_io();

int main( void ){
  return  0
      ||  test_basics()
      ||  test_parsers()
      ||  test_regex()
      ||  test_ebnf()
      ||  test_io()
      ;
}

static fOperator to_upper;

static integer
to_upper( object env, integer it ){
  return  Int( toupper( it->Int.i ) );
}

static int
test_basics(){
  puts( __func__ );
  list ch = chars_from_str( "abcdef" );
    print( ch ), puts("");
    print_list( ch ), puts("");
  integer a = apply( Operator( NIL_, to_upper ), first( ch ) );
    print( a ), puts("");
  drop( 1, a );
    print( a ), puts("");
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
  parser p = succeeds( Int('*') );
    print_list( parse( p, ch ) ), puts("");
  parser q = fails( String("Do you want a cookie?",0) );
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

static fOperator stringify;

static string
stringify( object env, list it ){
  return  to_string( it );
}

static int
test_ebnf(){
  puts( __func__ );
  Symbol(postal_address);
  Symbol(name_part);
  Symbol(street_address);
  Symbol(street_name);
  Symbol(zip_part);

  list parsers = ebnf(
    "postal_address = name_part street_address zip_part ;\n"
    "name_part = personal_part SP last_name SP opt_suffix_part EOL\n"
    "          | personal_part SP name_part ;\n"
    "personal_part = initial '.' | first_name ;\n"
    "street_address = house_num SP street_name opt_apt_num EOL ;\n"
    "zip_part = town_name ',' SP state_code SP zip_code EOL ;\n"
    "opt_suffix_part = 'Sr.' | 'Jr.' | roman_numeral | ;\n"
    "opt_apt_num = [ apt_num ] ;\n"
    "apt_num = NUMBER ;\n"
    "town_name = NAME ;\n"
    "state_code = UPPER UPPER ;\n"
    "zip_code = DIGIT DIGIT DIGIT DIGIT DIGIT ;\n"
    "initial = 'Mrs' | 'Mr' | 'Ms' | 'M' ;\n"
    "roman_numeral = 'I' [ 'V' | 'X' ] { 'I' } ;\n"
    "first_name = NAME ;\n"
    "last_name = NAME ;\n"
    "house_num = NUMBER ;\n"
    "street_name = NAME ;\n",
    env( NIL_, 6,
	 Symbol(EOL), chr('\n'),
	 Symbol(DIGIT), digit(),
	 Symbol(UPPER), upper(),
	 Symbol(NUMBER), some( digit() ),
         Symbol(NAME), some( alpha() ),
	 Symbol(SP), many( anyof( " \t\n" ) ) ),
    env( NIL_, 2,
         Symbol(name_part), Operator( NIL_, stringify ),
         Symbol(street_name), Operator( NIL_, stringify ) )
  );

  parser start = assoc_symbol( postal_address, parsers );
  if(  valid( start ) && start->t == LIST  )
    start = first( start );
  print_list( start ), puts("\n");
  
  print_list( parse( start,
      chars_from_str( "Mr. luser droog I\n2357 Streetname\nAnytown, ST 00700\n" ) ) ),
    puts("");

  printf( "%d objects\n", count_allocations() );
  return  0;
}

static int
test_io(){
  pprintf( "%s:%c-%c\n", "does it work?", '*', '@' );
  return  0;
}
