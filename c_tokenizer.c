#include "pc11ctoken.h"

static fSuspension force_tokens_from_chars;
char *lang_string( language lang );


list
tokens_from_chars( language lang, list input ){
  if(  ! valid( input )  ) return Symbol(EOF);
  return  cons( Symbol_( LANG_C90 + lang, lang_string( lang ), NIL_ ),
                Suspension( cons( Int(lang), input ), force_tokens_from_chars));
}

static list
force_tokens_from_chars( list env ){
  language lang = first( env )->Int.i;
  list input = rest( env );
  if(  ! valid( input )  ) return  Symbol(EOF);
}


#define Case_Lang_String(x) \
  case x: return Semantic_string( Language_name(x) );

char *
lang_string( language lang ){
  switch( lang ){
  default: return "";
  Languages( Case_Lang_String );
  }
}
