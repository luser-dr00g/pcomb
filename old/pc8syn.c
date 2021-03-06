#include "pc8syn.h"

#define Extra_Symbols(_) \
  _(t_id) _(c_int) _(c_float) _(c_char) _(c_string)

#define Parser_for_symbol_(n) parser n##_ = lit( Symbol(n) );
#define Parser_for_symbolic_(s,n) parser n##_ = lit( Symbol(n) );

parser parser_for_grammar( void ){
  Each_Symbolic( Parser_for_symbolic_ )
  Extra_Symbols( Parser_for_symbol_ )

  parser identifier = t_id_;
  parser asgnop     = PLUS( o_equal_, o_eplus_, o_eminus_, o_estar_, o_eslant_, o_epercent_,
			    o_egtgt_, o_eltlt_, o_eamp_, o_ecaret_, o_epipe_ );
  parser constant   = PLUS( c_int_, c_float_, c_char_, c_string_ );
  parser lvalue     = forward();
  parser expression = forward();

  *lvalue =
      *PLUS(
	  identifier,
	  seq( o_star_, expression ),
	  //SEQ( primary, o_arrow_, identifier ),  // introduces a left-recursion indirectly
	  SEQ( lparen_, lvalue, rparen_ )
      );

  parser expression_list = seq( expression, many( seq( comma_, expression ) ) );
  parser primary =
      seq(
	   PLUS(
	     identifier,
	     constant,
	     SEQ( lparen_, expression, rparen_ ),
	     SEQ( lvalue, o_dot_, identifier )
	   ),
	   maybe( PLUS(
	     SEQ( lparen_, expression_list, rparen_ ),
	     SEQ( lbrack_, expression, rbrack_ ),
	     seq( o_arrow_, identifier )
	   ) )
      );

  *expression =
      *seq(
	   PLUS(
	     primary,
	     seq( o_star_, expression ),
	     seq( o_amp_, expression ),
	     seq( o_minus_, expression ),
	     seq( o_bang_, expression ),
	     seq( o_tilde_, expression ),
	     seq( o_plusplus_, lvalue ),
	     seq( o_minusminus_, lvalue ),
	     seq( lvalue, o_plusplus_ ),
	     seq( lvalue, o_minusminus_ ),
	     seq( k_sizeof_, expression ),
	     SEQ( lvalue, asgnop, expression )
	   ),
	   maybe( PLUS(
			seq( PLUS( o_star_, o_slant_, o_percent_ ), expression ),
			seq( PLUS( o_plus_, o_minus_ ), expression ),
			seq( PLUS( o_ltlt_, o_gtgt_ ), expression ),
			seq( PLUS( o_lt_, o_le_, o_gt_, o_ge_ ), expression ),
			seq( PLUS( o_equalequal_, o_ne_ ), expression ),
			seq( o_amp_, expression ),
			seq( o_caret_, expression ),
			seq( o_pipe_, expression ),
			seq( o_ampamp_, expression ),
			seq( o_pipepipe_, expression ),
			SEQ( quest_, expression, colon_, expression ),
			seq( comma_, expression )
	   ) )
      );
  parser constant_expression = expression;

  parser statement  = forward();
  parser statement_list = many( statement );
        *statement  =
      *PLUS(
	    seq( expression, semi_ ),
	    SEQ( lbrace_, statement_list, rbrace_ ),
	    SEQ( k_if_, lparen_, expression, rparen_, statement ),
	    SEQ( k_if_, lparen_, expression, rparen_, statement, k_else_, statement ),
	    SEQ( k_do_, statement, k_while_, lparen_, expression, rparen_, semi_ ),
	    SEQ( k_while_, lparen_, expression, rparen_, statement ),
	    SEQ( k_for_, lparen_,
		   maybe( expression ), semi_, maybe( expression ), semi_, maybe( expression ),
		 rparen_, statement ),
	    SEQ( k_switch_, lparen_, expression, rparen_, statement ),
	    SEQ( k_case_, constant_expression, colon_, statement ),
	    SEQ( k_default_, colon_, statement ),
	    seq( k_break_, semi_ ),
	    seq( k_continue_, semi_ ),
	    seq( k_return_, semi_ ),
	    SEQ( k_return_, expression, semi_ ),
	    SEQ( k_goto_, expression, semi_ ),
	    SEQ( identifier, colon_, statement ),
	    semi_
      );

  parser type_specifier = forward();
  parser declarator_list = forward();
  parser type_declaration = SEQ( type_specifier, declarator_list, semi_ );
  parser type_decl_list = some( type_declaration );
  parser sc_specifier = PLUS( k_auto_, k_static_, k_extern_, k_register_ );
	*type_specifier = *PLUS(
				k_int_, k_char_, k_float_, k_double_,
				SEQ( k_struct_, lbrace_, type_decl_list, rbrace_ ),
				SEQ( k_struct_, identifier, lbrace_, type_decl_list, rbrace_ ),
				SEQ( k_struct_, identifier )
			       );

  parser declarator = forward();
	*declarator = *seq( PLUS(
			      identifier,
			      seq( o_star_, declarator ),
			      SEQ( lparen_, declarator, rparen_ )
			    ), maybe( PLUS(
					   seq( lparen_, rparen_ ),
					   SEQ( lbrack_, constant_expression, rbrack_ )
			    ) )
			  );
	*declarator_list = *seq( declarator, many( seq( comma_, declarator_list ) ) );
  parser decl_specifiers = PLUS( type_specifier, sc_specifier,
				 seq( type_specifier, sc_specifier ),
				 seq( sc_specifier, type_specifier ) );
  parser declaration = seq( decl_specifiers, maybe( declarator_list ) );
  parser declaration_list = forward();
	*declaration_list = *seq( declaration, maybe( seq( comma_, declaration_list ) ) );
  parser data_def = zero();

  parser parameter_list = forward();
	*parameter_list = *maybe( seq( expression, maybe( seq( comma_, parameter_list ) ) ) );
  parser function_declarator = SEQ( declarator, lparen_, maybe( parameter_list ), rparen_ );
  parser function_statement = SEQ( lbrace_, maybe( declaration_list ), many( statement ), rbrace_ );
  parser function_body = seq( type_decl_list, function_statement );
  parser function_def = SEQ( maybe( type_specifier ), function_declarator, function_body );

  parser external_def = plus( function_def, data_def );
  parser program = some( external_def );

  return  program;
}

list
tree_from_tokens( void *s ){
  if(  !s  ) return  NULL;
  static parser p;
  if(  !p  ){
    p = parser_for_grammar();
    add_global_root( p );
  }
  return  parse( p, s );
}

#include <stdio.h>

void print_flat( list a ){
  if(  !a  ) return;
  if(  a->t != LIST  ){ print( a ); return; }
  print_flat( a->List.a );
  print_flat( a->List.b );
}

void print_bare( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  case LIST: print_bare( a->List.a ), print_bare( a->List.b ); break;
  case STRING: printf("%s", a->String.string ); break;
  }
}

void print_data( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  case LIST:  print_data( a->List.a ), print_data( a->List.b ); break;
  case SYMBOL: print_bare( a->Symbol.data ); break;
  }
}

void print_list();
void print_listn( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  default: print( a ); return;
  case LIST: print_list( x_( a ) ), print_listn( xs_( a ) ); return;
  }
}

void print_list( list a ){
  if(  !a  ) return;
  switch(  a->t  ){
  default: print( a ); return;
  case LIST: printf( "(" ), print_list( x_( a ) ), print_listn( xs_( a ) ), printf( ")" ); return;
  }
}

#define PRINT(__) printf( "%s =\n", #__ ), print( __ ), puts("")
#define PRINT_FLAT(__) printf( "%s =\n", #__ ), print_flat( __ ), puts("")
#define PRINT_DATA(__) printf( "%s =\n", #__ ), print_data( __ ), puts("")
#define PRINT_LIST(__) printf( "%s =\n", #__ ), print_list( __ ), puts("")
int test_parser(){
  char *source =
"int max(a, b, c)\n"
"int a, b, c;\n"
"{\n"
"      int m;\n"
"      m = (a>b)? a:b;\n"
"      return(m>c? m:c);\n"
"}\n"
"\t if(  2  ){\n\t   x = 5;\n\t   } int auto";
  object tokens = tokens_from_chars( chars_from_string( source ) );
  add_global_root( tokens );
  PRINT( take( 4, tokens ) );
  object program = tree_from_tokens( tokens );
  //PRINT( program );
  printf( "gc: %d\n", garbage_collect( program ) );
  PRINT( x_( x_( program ) ) );
  PRINT_FLAT( x_( x_( program ) ) );
  PRINT_LIST( x_( x_( program ) ) );
  PRINT_DATA( x_( x_( program ) ) );
  PRINT( xs_( x_( program ) ) );
}

int main(){ test_parser(); }
