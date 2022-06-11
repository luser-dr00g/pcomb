#include "pc11parser.h"
static int test_basics();
static int test_parsers();
static int test_regex();

int main( void ){
  return  test_basics()
      ||  test_parsers()
      ||  test_regex();
}

static int
test_basics(){
  puts( "test_basics" );
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
  puts( "test_parsers" );
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
  puts( "test_regex" );
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
  puts("");
  return  0;
}
