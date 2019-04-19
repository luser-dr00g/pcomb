#include "pc8tok.h"

static object  on_spaces( void *v, list o ){ return  string_from_chars( o ); }

static object  on_integer( void *v, list o ){ return  cons( Symbol(c_int), string_from_chars( o ) ); }
static object  on_floating( void *v, list o ){ return  cons( Symbol(c_float), string_from_chars( o ) ); }
static object  on_character( void *v, list o ){ return  cons( Symbol(c_char), string_from_chars( o ) ); }
static object  on_string( void *v, list o ){ return  cons( Symbol(c_string), string_from_chars( o ) ); }
static object  on_identifier( void *v, list o ){ return  cons( Symbol(t_id), string_from_chars( o ) ); }

#define On_Symbolic(x,y) \
  static object  on_##y( void *v, list o ){ return  cons( Symbol(y), string_from_chars( o ) ); }
Each_Symbolic( On_Symbolic )

static object  on_token( void *v, list o ){
  object space = x_( o );
  object symbol = x_( xs_( o ) );
  object string = xs_( xs_( o ) );
  return  symbol->Symbol.data = cons( space, string ),  symbol;
}

#define Handle_Symbolic(x,y) using( str( x ), on_##y ),
list
tokens_from_chars( void *s ){
  if(  !s  ) return  NULL;
  static parser p;
  if(  !p  ){
    parser space      = using( many( anyof( " \t\n" ) ), on_spaces );
    parser alpha_     = plus( alpha(), chr('_') );
    parser integer    = using( some( digit() ), on_integer );
    parser floating   = using( SEQ( plus( SEQ( integer, chr('.'), maybe( integer ) ),
                                         SEQ( chr('.'), integer ) ),
                                   maybe( SEQ( chr('e'), maybe( plus( chr('+'), chr('-') ) ), integer ) ) ),
                              on_floating );
    parser escape     = seq( chr('\\'),
                            plus( seq( digit(), maybe( seq( digit(), maybe( digit() ) ) ) ),
                                  anyof( "'\"bnrt\\" ) ) );
    parser character  = using( xthen( chr('\''), thenx( plus( escape, item() ), chr('\'') ) ), on_character );
    parser string     = using( SEQ( chr('"'), many( character ), chr('"') ), on_string );
    parser constant   = PLUS( integer, floating, character, string );
    parser symbolic   = PLUS( Each_Symbolic( Handle_Symbolic ) zero() );
    parser identifier = using( seq( alpha_ , many( plus( alpha_, digit() ) ) ), on_identifier );
    p = using( seq( space, PLUS( constant, symbolic, identifier ) ), on_token );
    add_global_root( p );
  }
  list r = x_( parse( p, s ) );
  return  cons( x_( r ), Suspension( xs_( r ), tokens_from_chars ) );
}

/*
#include <stdio.h>

int test_tokens(){
  list tokens = tokens_from_chars( chars_from_string( "auto ;*++'\\42' '\\n' 123 if" ) );
  print( take( 3, tokens ) ), puts("");
  print( drop( 3, tokens ) ), puts("");
  print( take( 2, drop( 3, tokens ) ) ), puts("");
}
/**/
//int main(){ test_tokens(); }
