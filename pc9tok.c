#include "pc9tok.h"
#include "pc9objpriv.h"

static object  on_spaces( object v, list o ){ return  string_from_chars( o ); }

static object  on_integer( object v, list o ){ return  cons( Symbol(c_int), string_from_chars( o ) ); }
static object  on_floating( object v, list o ){ return  cons( Symbol(c_float), string_from_chars( o ) ); }
static object  on_character( object v, list o ){ return  cons( Symbol(c_char), string_from_chars( o ) ); }
static object  on_string( object v, list o ){ return  cons( Symbol(c_string), string_from_chars( o ) ); }
static object  on_identifier( object s, list o ){ return  cons( Symbol(t_id), string_from_chars( o ) ); }

#define On_Symbolic(a,b) \
  static object  on_##b( object v, list o ){ return  cons( Symbol(b), string_from_chars( o ) ); }
Each_Symbolic( On_Symbolic )
Each_C75_assignop( On_Symbolic )
Each_assignop( On_Symbolic )

static parser
token_parser( language lang ){
  parser space      = using( many( anyof( " \t\n" ) ), 0, on_spaces );
  parser alpha_     = plus( alpha(), chr('_') );
  parser integer    = using( some( digit() ), 0, on_integer );
  parser floating   = using( SEQ( plus( SEQ( some( digit() ), chr('.'), many( digit() ) ),
                                 seq( chr('.'), some( digit() ) ) ),
                                 maybe( SEQ( anyof("eE"), maybe( anyof("+-") ), some( digit() ) ) ) ),
                             0, on_floating );
  parser escape     = seq( chr('\\'),
                           plus( seq( digit(), maybe( seq( digit(), maybe( digit() ) ) ) ),
                                 anyof( "'\"bnrt\\" ) ) );
  parser char_      = plus( escape, noneof( "'\n" ) );
  parser schar_     = plus( escape, noneof( "\"\n" ) );
  parser character  = using( SEQ( chr('\''), char_, chr('\'') ), 0, on_character );
  parser string     = using( SEQ( chr('"'), many( schar_ ), chr('"') ), 0, on_string );
  parser constant   = PLUS( floating, integer, character, string );
# define Handle_Symbolic(a,b)  using( str( a ), 0, on_##b ),
  parser assignop75 = PLUS( Each_C75_assignop( Handle_Symbolic ) zero() );
  parser assignop   = PLUS( Each_assignop( Handle_Symbolic )
                            zero() );
  parser symbolic   = PLUS( Each_Symbolic( Handle_Symbolic )
                            lang==C75  ? assignop75  : assignop );
  parser identifier = using( seq( alpha_, many( plus( alpha_, digit() ) ) ), 0, on_identifier );
  return  seq( space, PLUS( constant, symbolic, identifier ) );
}

static object  on_token( object v, list o ){
  object space = x_( o );
  object symbol = x_( xs_( o ) );
  object string = xs_( xs_( o ) );
  return  symbol->Symbol.data = cons( space, string ),  symbol;
  return  cons( symbol, cons( space, string ) );
}

static list
force_tokens_from_chars( object v ){
  object ilang = x_( v );
  language lang = ilang->Int.i;
  object s = xs_( v );
  if(  !valid( s )  ) return  Symbol(EOF);
  static parser p;
  static language plang;
  if(  !p  || lang != plang ){
    p = using( token_parser( lang ), 0, on_token );
    plang = lang;
    add_global_root( p );
  }
  list r = parse( p, s );
  take( 1, r );
  r = x_( r );
  return  cons( x_( r ),
                Suspension( cons( ilang, xs_( r ) ),
                            force_tokens_from_chars ) );
}

list
tokens_from_chars( language lang, object s ){
  return  valid( s )  ? Suspension( cons( Int(lang), s ), force_tokens_from_chars )
                      : Symbol(EOF);
}


int test_tokens(){
  list tokens = tokens_from_chars( C75, 
                chars_from_string( "'x' auto \"abc\" 12 ;*++'\\42' '\\n' 123 if" ) );
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
