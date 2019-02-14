#include <stdio.h>
#include "pc4re.h"

void print( void *p, char *s ){ printf("%s\n", s); }

int simple_test(){
  parser digit = char_class( "0123456789" );
  parser number = some( digit );
  parser integer = sequence( maybe( char_class( "+-" ) ), number );
  parser on_integer = action( integer, print, NULL );

  result r = parse( on_integer, "123" );
  printf( "%d\n", r ? r->length_matched : -1 );
}

int try(parser x, char *s){
  printf( "%s -> ", s );
  result r = parse( x, s );
  printf( "%d\n", r ? r->length_matched : -1 );
}

int regex_test(){
  parser x = regex( "ab?" );
  try( x, "a" );
  try( x, "ab" );
  try( x, "abx" );
  try( x, "abxb" );
  try( x, "abxx" );
  try( x, "abxbx" );
  try( x, "abxabx" );
  try( x, "axbxx" );
}

int main(){
  regex_test();
  return 0;
}




#if 0
int ast_test(){
  parser digit = char_class( "0123456789" );
  parser number = some( digit );
  parser integer = sequence( maybe( char_class( "+-" ) ), number );
  parser on_integer = action( integer, leaf, token );
}

int my_sscanf( const char * restrict str, const char *restrict format, ...){
  static parser p = NULL;
  if(  !p  ) p = 0;
}

#endif

#if 0
parser c_grammar(){
  parser constant_expression;
  parser comma;
  parser identifier;
  parser left_brace;
  parser right_brace;
  parser enum_specifier;
  parser typedef_name;
  parser compound_statement;
  parser declaration;
  parser direct_declarator;/* = alternaten(5, (parser[]){
      identifier,
	sequencen(3, (parser[]){ string("("), declarator, string(")") }),
	sequencen(4, (parser[]){ direct_declarator, */

  parser type_qualifier = alternate( string("const"), string("volatile") );
  parser pointer = sequencen(3, (parser[]){ string("*"), many( type_qualifier ), maybe( pointer ) });
  parser declarator = sequence( maybe( pointer ), direct_declarator );
  parser struct_declarator = alternaten(3, (parser[]){
      declarator,
      sequencen(3, (parser[]){ declarator, string(":"), constant_expression }),
      sequence( string(":"), constant_expression )
	});
  parser struct_declarator_list = sequence( struct_declarator,
					    many( sequence( comma, struct_declarator ) ) );
  parser type_specifier = alternaten(12, (parser[]){
      string("void"), string("char"), string("short"), string("int"), string("long"),
      string("float"), string("double"), string("signed"), string("unsigned"),
      struct_or_union_specifier, enum_specifier, typedef_name
	});
  parser specifier_qualifier = alternate( type_specifier, type_qualifier );
  parser struct_declaration = sequence( many( specifier_qualifier ), struct_declarator_list );
  parser struct_or_union = alternate( string("struct"), string("union") );
  parser struct_or_union_specifier = alternaten(3, (parser[]){
      sequencen(5, (parser[]){
          struct_or_union, identifier, left_brace, some( struct_declaration ), right_brace
	  } ),
      sequencen(4, (parser[]){ struct_or_union, left_brace, some( struct_declaration ), right_brace } ),
      sequence( struct_or_union, identifier )
	});
  parser storage_class_specifier = alternaten(5, (parser[]){
      string("auto"), string("register"), string("static"), string("extern"), string("typedef")
	});
  parser declaration_specifier = alternaten(3, (parser[]){
      storage_class_specifier, type_specifier, type_qualifier
	});
  parser function_definition = sequencen(4, (parser[]){
      many( declaration_specifier ), declarator, many( declaration ), compound_statement
	});
  parser external_declaration = alternate( function_definition, declaration );
  parser translation_unit = many( external_declaration );
  return translation_unit;
}
#endif
  
