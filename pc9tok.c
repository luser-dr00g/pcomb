#include "pc9tok.h"
#include "pc9objpriv.h"

static object  on_spaces( void *v, list o ){ return  string_from_chars( o ); }

static object  on_integer( void *v, list o ){ return  cons( Symbol(c_int), string_from_chars( o ) ); }
static object  on_floating( void *v, list o ){ return  cons( Symbol(c_float), string_from_chars( o ) ); }
static object  on_character( void *v, list o ){ return  cons( Symbol(c_char), string_from_chars( o ) ); }
static object  on_string( void *v, list o ){ return  cons( Symbol(c_string), string_from_chars( o ) ); }
static object  on_identifier( void *s, list o ){ return  cons( Symbol(t_id), string_from_chars( o ) ); }

#define On_Symbolic(a,b) \
  static object  on_##b( void *v, list o ){ return  cons( Symbol(b), string_from_chars( o ) ); }
Each_Symbolic( On_Symbolic )

static parser
token_parser( void ){
  parser space      = using( many( anyof( " \t\n" ) ), on_spaces );
  parser alpha_     = plus( alpha(), chr('_') );
  parser integer    = using( some( digit() ), on_integer );
  parser floating   = using( SEQ( plus( SEQ( some( digit() ), chr('.'), many( digit() ) ),
                                 seq( chr('.'), some( digit() ) ) ),
                                 maybe( SEQ( anyof("eE"), maybe( anyof("+-") ), some( digit() ) ) ) ),
                             on_floating );
  parser escape     = seq( chr('\\'),
                           plus( seq( digit(), maybe( seq( digit(), maybe( digit() ) ) ) ),
                                 anyof( "'\"bnrt\\" ) ) );
  parser char_      = plus( escape, noneof( "'\n" ) );
  parser schar_     = plus( escape, noneof( "\"\n" ) );
  parser character  = using( SEQ( chr('\''), char_, chr('\'') ), on_character );
  parser string     = using( SEQ( chr('"'), many( schar_ ), chr('"') ), on_string );
  parser constant   = PLUS( integer, floating, character, string );
# define Handle_Symbolic(a,b)  using( str( a ), on_##b ),
  parser symbolic   = PLUS( Each_Symbolic( Handle_Symbolic ) zero() );
  parser identifier = using( seq( alpha_, many( plus( alpha_, digit() ) ) ), on_identifier );
  return  seq( space, PLUS( constant, symbolic, identifier ) );
}

static object  on_token( void *v, list o ){
  object space = x_( o );
  object symbol = x_( xs_( o ) );
  object string = xs_( xs_( o ) );
  return  symbol->Symbol.data = cons( space, string ),  symbol;
  return  cons( symbol, cons( space, string ) );
}

list
ptokens_from_chars( void *s ){
  if(  !valid( s )  ) return  Symbol(EOF);
  static parser p;
  if(  !p  ){
    p = using( token_parser(), on_token );
    add_global_root( p );
  }
  list r = parse( p, s );
  take( 1, r );
  //PRINT( r );
  r = x_( r );
  return  cons( x_( r ), Suspension( xs_( r ), ptokens_from_chars ) );
}

list
tokens_from_chars( void *s ){
  return  valid( s )  ? Suspension( s, ptokens_from_chars )  : Symbol(EOF);
}


int test_tokens(){
  list tokens = tokens_from_chars( chars_from_string( "'x' auto \"abc\" 12 ;*++'\\42' '\\n' 123 if" ) );
  PRINT( tokens );
  PRINT( take( 1, tokens ) );
  PRINT( take( 2, tokens ) );
  PRINT( drop( 1, tokens ) );
  PRINT( take( 2, drop( 1, tokens ) ) );
  drop( 7, tokens );
  PRINT( tokens );
  PRINT( Int( garbage_collect( tokens ) ) );
  return 0;
}

int tok_main(){
  return
          par_main(),
          test_tokens(),
          0;
}
