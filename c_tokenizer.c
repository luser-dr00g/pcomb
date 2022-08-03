#include "c_tokenizer.h"



#define Declare_Semantic_Op_fun( x ) \
        Declare_Op_fun( 0, x )

#define Declare_Op_fun( x, y ) \
  static fOperator on_ ## y ;


#define Semantic_Op_fun( x ) \
        Op_fun( 0, x )

#define Op_fun( x, y ) \
  static object \
  on_ ## y ( object v, list input ){ \
    return  Symbol_( y, #y, to_string( input ) ); \
  }



static fSuspension force_tokens_from_chars;
static parser token_parser( language lang );
char *lang_string( language lang );

Each_Symbolic( Declare_Op_fun )
Each_Assignop( Declare_Op_fun )
Semantic_Tokens( Declare_Semantic_Op_fun )

static fOperator on_space;
static fOperator on_token;



list
tokens_from_chars( language lang, list input ){
  if(  ! valid( input )  ) return Symbol(EOF);
  return  cons( Symbol_( LANG_C90 + lang, lang_string( lang ), NIL_ ),
                Suspension( cons( Int(lang), input ),
			    force_tokens_from_chars ) );
}


static list
force_tokens_from_chars( list env ){
  integer intlang = first( env );
  language lang = intlang->Int.i;
  list input = rest( env );
  if(  ! valid( input )  ) return  Symbol(EOF);

  static parser p;
  static language plang;
  if(  ! p  || lang != plang  )
    p = token_parser( lang ), plang = lang;

  list result = parse( p, input );
  if(  not_ok( result )  ) return result;

  list payload = rest( result );
  object value = first( payload );
  list remainder = rest( payload );
  return  cons( value,
		Suspension( cons( intlang, remainder ),
			    force_tokens_from_chars ) );
}


static parser
token_parser( language lang ){
  parser comment  = SEQ( str("/*"),
			 some( then( many( noneof("*") ), chr('*') ) ),
			 chr('/') );
  parser space = many( either( anyof(" \t\n"), comment ) );
  parser alpha_ = either( alpha(), chr('_') );
  parser int_ = some( digit() );
  parser float_ = then( either( SEQ( some( digit() ),
				     chr('.'),
				     many( digit() ) ),
			        then( chr('.'), some( digit() ) ) ),
		        maybe( SEQ( anyof("Ee"),
				    maybe( anyof("+-") ),
				    some( digit() ) ) ) );
  parser escape = then( chr('\\'),
			either( then( digit(),
				      maybe( then( digit(),
						   maybe( digit() ) ) ) ),
				anyof( "'\"bnrt\\" ) ) );
  parser char_ = either( escape, noneof( "'\n" ) );
  parser schar_ = either( escape, noneof( "\"\n" ) );
  parser character = SEQ( chr('\''), char_, chr('\'') );
  parser string_ = SEQ( chr('"'), many( schar_ ), chr('"') );
  parser constant = ANY( bind( float_, Operator( NIL_, on_const_float ) ),
			 bind( int_, Operator( NIL_, on_const_int ) ),
			 bind( character, Operator( NIL_, on_const_char ) ),
			 bind( string_, Operator( NIL_, on_const_string ) ) );

#define Parser_For_Symbolic_comma( x, y ) \
        bind( str(x), Operator( NIL_, on_ ## y ) ) ,

  parser assign = ANY( Each_Assignop( Parser_For_Symbolic_comma )
		       fails(NIL_) );
  parser symbolic = ANY( Each_Symbolic( Parser_For_Symbolic_comma )
			 assign );
  parser identifier = bind( then( alpha_, many( either( alpha_, digit() ) ) ),
			    Operator( NIL_, on_ident ) );
  return  into( space, Symbol(SPACE),
		bind( ANY( constant, symbolic, identifier ),
		      Operator( NIL_, on_token )) );
}


#define Case_Lang_String( x ) \
  case x: return Semantic_string( Language_name(x) );

char *
lang_string( language lang ){
  switch( lang ){
  default: return  "";
  Languages( Case_Lang_String );
  }
}


Each_Symbolic( Op_fun )
Each_Assignop( Op_fun )
Semantic_Tokens( Semantic_Op_fun )

static string
on_space( object v, list input ){
  return  to_string( input );
}

static symbol
on_token( object v, symbol input ){
  input->Symbol.data = cons( assoc_symbol( SPACE, v ),
			     input->Symbol.data );
  return  input;
}
