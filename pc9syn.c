#include <string.h>
#include "pc9syn.h"
#include "pc9objpriv.h"

#define Semantic_Tokens(_) \
  _(t_id) _(c_int) _(c_float) _(c_char) _(c_string)

#define Parser_for_symbolic_(a,b)  parser b##_ = lit( Symbol(b) );
#define Parser_for_token_(b)      parser b##_ = lit( Symbol(b) );

#define FLAT(p) using( p, 0, flatten )
#define ANN(sym, p) using( p, 0, on_##sym );

list listify( object a, object b ){
  return  a && a->t == LIST  ? cons( x_(a), listify( xs_(a), b ) )  :
          b && b->t == LIST  ? cons( a, b )  :
          cons( a, cons( b, 0 ) );
}
object flatten( object v, list o ){ return  collapse( listify, o ); }
//object flatten( object v, list o ){ return  collapse( cons, o ); }
//object flatten( object v, list o ){ return  collapse( append, o ); }

object pass_through(   object sym, list o ){ return  o; }
object prepend_symbol( object sym, list o ){ return  cons( sym, o ); }
object embed_data(     object sym, list o ){ return  sym->Symbol.data = o, sym; }
object (*syntax_annotation)( object, list ) = embed_data;

#define Annotations(_) \
 _(func_def) _(data_def) _(type_spec) _(body) _(statement) _(expr) \
 _(decl_list)

#define Handler_for_annotation(a)             \
static object on_##a( object v, list o ){     \
  return  syntax_annotation( Symbol(a), o );  \
}

Annotations( Handler_for_annotation )


static parser
parser_for_c_grammar( void ){
  Each_Symbolic( Parser_for_symbolic_ )
  Each_C75_assignop( Parser_for_symbolic_ )
  Semantic_Tokens( Parser_for_token_ )

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
      *ANN( expr, seq(
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
      ) );
  parser constant_expression = expression;

  parser statement  = forward();
  parser statement_list = many( statement );
        *statement  =
	    *ANN( statement, PLUS(
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
	    ) );

  parser constant_expression_list = seq( constant_expression, many( seq( comma_, constant_expression ) ) );
  parser initializer = plus( constant, constant_expression_list );

  parser type_specifier = forward();
  parser declarator_list = forward();
  parser type_declaration = SEQ( type_specifier, declarator_list, semi_ );
  parser type_decl_list = ANN( decl_list, some( type_declaration ) );
  parser sc_specifier = PLUS( k_auto_, k_static_, k_extern_, k_register_ );
	*type_specifier = *ANN( type_spec, PLUS(
				k_int_, k_char_, k_float_, k_double_,
				SEQ( k_struct_, lbrace_, type_decl_list, rbrace_ ),
				SEQ( k_struct_, identifier, lbrace_, type_decl_list, rbrace_ ),
				SEQ( k_struct_, identifier )
			       ) );

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
  parser decl_specifiers = PLUS( type_specifier, 
                                 sc_specifier,
				 seq( type_specifier, sc_specifier ),
				 seq( sc_specifier, type_specifier ) );
  parser declaration = seq( decl_specifiers, maybe( declarator_list ) );
  parser declaration_list = SEQ( declaration, many( seq( comma_, declaration ) ), semi_ );
  parser init_declarator = seq( declarator, maybe( initializer ) );
  parser init_declarator_list = seq( FLAT( init_declarator ), many( FLAT( seq( comma_, init_declarator ) ) ) );
  parser data_def = ANN( data_def, SEQ( maybe( k_extern_ ),
                                maybe( type_specifier ),
                                maybe( init_declarator_list ), semi_ ) );

  parser parameter_list = maybe( seq( expression, many( seq( comma_, expression ) ) ) );
  parser function_declarator = SEQ( declarator, lparen_, parameter_list, rparen_ );
  parser function_statement = SEQ( lbrace_,
                                   maybe( declaration_list ),
                                   many( statement ),
                                   rbrace_ );
  parser function_body = ANN( body, seq( maybe( type_decl_list ),
                                     function_statement ) );
  parser function_def = ANN( func_def, SEQ( maybe( type_specifier ),
                                    function_declarator,
                                    function_body ) );

  parser external_def = plus( function_def, data_def );
  parser program = seq( lit( Symbol(LANG_C75) ), some( external_def ) );

  return  program;
}

list
tree_from_tokens( object s ){
  if(  !s  ) return  NIL_;
  static parser p;
  if(  !p  ){
    p = parser_for_c_grammar();
    add_global_root( p );
  }
  return  parse( p, s );
}


#define Test_for_symbol(b)  || !strcmp(pname, #b)

static int
annotationq( char *pname ){
  return  0  Annotations( Test_for_symbol );
}

static int
semantictokenq( char *pname ){
  return  0  Semantic_Tokens( Test_for_symbol );
}


list
suppress_strings( list a ){
  if(  !a  ) return  a;
  switch(  a->t  ){
  case LIST: return  cons( suppress_strings( a->List.a ),
                           suppress_strings( a->List.b ) );
  case SYMBOL: return
    Symbol_( a->Symbol.symbol, a->Symbol.pname, 
        annotationq( a->Symbol.pname ) ? suppress_strings( a->Symbol.data ) :
        semantictokenq( a->Symbol.pname ) ? xs_( a->Symbol.data ): 0 );
  }
  return  a;
}


static int
ast_keep( char *pname ){
  return  annotationq( pname )
      ||  semantictokenq( pname )
      ||  !strcmp( pname, "quest" )
      ||  !strcmp( pname, "colon" )
//      ||  !strcmp( pname, "lbrace" )
//      ||  !strcmp( pname, "rbrace" )
      ||  !strncmp( pname, "o_", 2 )
      ||  !strncmp( pname, "k_", 2 )
      ||  0;
}

list
ast_from_tree( list a ){
  if(  !a  ) return  a;
  switch(  a->t  ){
  case LIST: return  cons( ast_from_tree( a->List.a ),
                           ast_from_tree( a->List.b ) );
  case SYMBOL: return
    ast_keep( a->Symbol.pname )  ?
      Symbol_( a->Symbol.symbol, a->Symbol.pname,
               annotationq( a->Symbol.pname )  ? ast_from_tree( a->Symbol.data )  :
               ast_keep( a->Symbol.pname )  ? a->Symbol.data  : 0 )
      : 0;
  }
  return  a;
}


list
structure_from_ast( list a ){
  if(  !a  ) return  a;
  switch(  a->t  ){
  case LIST: return  cons( structure_from_ast( a->List.a ),
                           structure_from_ast( a->List.b ) );
  case SYMBOL: return
    annotationq( a->Symbol.pname )  ?
      Symbol_( a->Symbol.symbol, a->Symbol.pname,
               annotationq( a->Symbol.pname )  ? structure_from_ast( a->Symbol.data )  :
	       a->Symbol.data )
      : 0;
  }
  return  a;
}


list
prune_twigs( list a ){
  if(  !a  ) return  a;
  switch(  a->t  ){
  case LIST: {
    object x = prune_twigs( x_( a ) );
    object y = prune_twigs( xs_( a ) );
    return  x && y  ? cons( x , y )  :
            x       ? x  : y;
    }
  case SYMBOL: return
      Symbol_( a->Symbol.symbol, a->Symbol.pname,
               annotationq( a->Symbol.pname )  ? prune_twigs( a->Symbol.data )  :
	       a->Symbol.data );
  }
  return  a;
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
  object tokens = tokens_from_chars( C75, chars_from_string( source ) );
  add_global_root( tokens );
  PRINT( take( 4, tokens ) );
  object program = tree_from_tokens( tokens );
  PRINT( Int( garbage_collect( program ) ) );
  PRINT( program );
  PRINT( x_( x_(  ( drop( 1, program ), program ) ) ) );
  PRINT_DOT( x_( x_( program ) ) );
  PRINT_FLAT( x_( x_( program ) ) );
  PRINT_DATA( x_( x_( program ) ) );
  PRINT_TREE( x_( x_( program ) ) );
  PRINT( xs_( x_( program ) ) );
  PRINT_TREE( program = suppress_strings( x_( x_( program ) ) ) );
  PRINT_DOT( program );
  PRINT( Int( garbage_collect( program ) ) );
  PRINT_TREE( program = ast_from_tree( program ) );
  PRINT_DOT( program );
  PRINT_TREE( program = prune_twigs( program ) );
  PRINT_DOT( program );
  PRINT_TREE( program = structure_from_ast( program ) );
  PRINT_DOT( program );
  PRINT( Int( garbage_collect( program ) ) );
  return  0;
}


int syn_main(){
  return  //tok_main(),
          test_syntax(),
          0;
}
