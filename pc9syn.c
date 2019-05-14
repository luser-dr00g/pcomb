#include "pc9syn.h"
#include "pc9objpriv.h"

#define Extra_Symbols(_) \
  _(t_id) _(c_int) _(c_float) _(c_char) _(c_string)

#define Parser_for_symbolic_(a,b)  parser b##_ = lit( Symbol(b) );
#define Parser_for_symbol_(b)      parser b##_ = lit( Symbol(b) );

static object on_func_def( object v, list o ){ 
  object s = Symbol(func_def); return  s->Symbol.data = o, s;
  return  cons( Symbol(func_def), o ); 
}
static object on_data_def( object v, list o ){
  object s = Symbol(data_def); return  s->Symbol.data = o, s;
}

parser
parser_for_grammar( void ){
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

  parser constant_expression_list = seq( constant_expression, many( seq( comma_, constant_expression ) ) );
  parser initializer = plus( constant, constant_expression_list );

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
	*declarator_list = *seq( declarator, many( seq( comma_, declarator ) ) );
  parser decl_specifiers = PLUS( type_specifier, sc_specifier,
				 seq( type_specifier, sc_specifier ),
				 seq( sc_specifier, type_specifier ) );
  parser declaration = seq( decl_specifiers, maybe( declarator_list ) );
  parser declaration_list = seq( declaration, many( seq( comma_, declaration ) ) );
  parser init_declarator = seq( declarator, maybe( initializer ) );
  parser init_declarator_list = seq( init_declarator, many( seq( comma_, init_declarator ) ) );
  parser data_def = using( SEQ( maybe( k_extern_ ),
                                maybe( type_specifier ),
                                maybe( init_declarator_list ), semi_ ),
                           on_data_def );

  parser parameter_list = maybe( seq( expression, many( seq( comma_, expression ) ) ) );
  parser function_declarator = SEQ( declarator, lparen_, parameter_list, rparen_ );
  parser function_statement = SEQ( lbrace_, maybe( declaration_list ), many( statement ), rbrace_ );
  parser function_body = seq( maybe( type_decl_list ), function_statement );
  parser function_def = using( SEQ( maybe( type_specifier ), function_declarator, function_body ),
                               on_func_def );

  parser external_def = plus( function_def, data_def );
  parser program = some( external_def );

  return  program;
}

list
tree_from_tokens( object s ){
  if(  !s  ) return  NIL_;
  static parser p;
  if(  !p  ){
    p = parser_for_grammar();
    add_global_root( p );
  }
  return  parse( p, s );
}

int test_syntax(){
  char *source =
"\n"
"int i,j,k 5;\n"
"float d 3.4;\n"
"int max(a, b, c)\n"
"int a, b, c;\n"
"{\n"
"      int m;\n"
"      m = (a>b)? a:b;\n"
"      return(m>c? m:c);\n"
"}\n"
"main( ) {\n"
"\tprintf(\"Hello, world\");\n"
"}\n"
"\t if(  2  ){\n\t   x = 5;\n\t   } int auto";
  object tokens = tokens_from_chars( chars_from_string( source ) );
  add_global_root( tokens );
  PRINT( take( 4, tokens ) );
  object program = tree_from_tokens( tokens );
  PRINT( program );
  PRINT( x_( x_(  ( drop( 1, program ), program ) ) ) );
  PRINT_FLAT( x_( x_( program ) ) );
  PRINT_DATA( x_( x_( program ) ) );
  PRINT( xs_( x_( program ) ) );
  PRINT( Int( garbage_collect( program ) ) );
  return  0;
}


int main(){
  return  tok_main(),
          test_syntax(),
          0;
}
