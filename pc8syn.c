#include "pc8syn.h"


#define Each_Symbol(_) \
  _(t_id) _(c_int) _(c_float) _(c_char) _(c_string) \
  _(comma) _(lparen) _(rparen) _(lbrace) _(rbrace) _(lbrack) _(rbrack) _(semi) _(quest) _(colon) \
  _(o_star) _(o_amp) _(o_plus) _(o_plusplus) _(o_minus) _(o_minusminus) _(o_equal) \
  _(o_bang) _(o_tilde) _(o_dot) _(o_arrow) _(o_slant) _(o_percent) \
  _(o_ltlt) _(o_gtgt) _(o_lt) _(o_le) _(o_gt) _(o_ge) _(o_equalequal) _(o_ne) \
  _(o_caret) _(o_pipe) _(o_ampamp) _(o_pipepipe) \
  _(o_eplus) _(o_eminus) _(o_estar) _(o_eslant) _(o_epercent) \
  _(o_egtgt) _(o_eltlt) _(o_eamp) _(o_ecaret) _(o_epipe) \
  _(k_if) _(k_else) _(k_sizeof) _(k_break) _(k_continue) _(k_return) \
  _(k_do) _(k_while) _(k_for) _(k_switch) _(k_case) _(k_default) _(k_goto) \
  _(k_auto) _(k_extern) _(k_static) _(k_register) \
  _(k_int) _(k_char) _(k_float) _(k_double) _(k_struct) 

#define Parser_for_symbol_(n) parser n##_ = lit( Symbol(n) );
list
tree_from_tokens( void *s ){
  if(  !s  ) return  NULL;
  static parser p;
  if(  !p  ){
    Each_Symbol( Parser_for_symbol_ )

    parser identifier = t_id_;
    parser asgnop     = PLUS( o_equal_, o_eplus_, o_eminus_, o_estar_, o_eslant_, o_epercent_,
                              o_egtgt_, o_eltlt_, o_eamp_, o_ecaret_, o_epipe_ );
    parser constant   = PLUS( c_int_, c_float_, c_char_, c_string_ );
    parser lvalue     = forward();
    parser expression = forward();

    parser lvalue_ =
        PLUS(
	    identifier,
	    seq( o_star_, expression ),
	    //SEQ( primary, o_arrow_, identifier ),
	    SEQ( lparen_, lvalue, rparen_ )
	);
    *lvalue = *lvalue_;

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

    parser expression_ =
        seq(
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
    *expression = *expression_;
    parser constant_expression = expression;

    parser statement  = forward();
    parser statement_list = many( statement );
    parser statement_ =
        PLUS(
	      seq( expression, semi_ ),
	      SEQ( lbrace_, statement_list, rbrace_ ),
	      SEQ( k_if_, lparen_, expression, rparen_, statement ),
	      SEQ( k_if_, lparen_, expression, rparen_, statement, k_else_, statement ),
              SEQ( k_do_, statement, k_while_, lparen_, expression, rparen_, semi_ ),
	      SEQ( k_while_, lparen_, expression, rparen_, statement ),
	      SEQ( k_for_, lparen_,
		     maybe( expression ), semi_,
		     maybe( expression ), semi_,
		     maybe( expression ),
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
    *statement  = *statement_;

    parser type_specifier = forward();
    parser declarator_list = forward();
    parser type_declaration = SEQ( type_specifier, declarator_list, semi_ );
    parser type_decl_list = some( type_declaration );
    parser sc_specifier = PLUS( k_auto_, k_static_, k_extern_, k_register_ );
    parser type_specifier_= PLUS(
                                  k_int_, k_char_, k_float_, k_double_,
                                  SEQ( k_struct_, lbrace_, type_decl_list, rbrace_ ),
                                  SEQ( k_struct_, identifier, lbrace_, type_decl_list, rbrace_ ),
                                  SEQ( k_struct_, identifier )
                                );
          *type_specifier = *type_specifier_;
    parser declarator = forward();
    parser declarator_= seq( PLUS(
                              identifier,
                              seq( o_star_, declarator ),
                              SEQ( lparen_, declarator, rparen_ )
                             ), maybe( PLUS(
                                             seq( lparen_, rparen_ ),
                                             SEQ( lbrack_, constant_expression, rbrack_ )
                             ) )
                           );
          *declarator = *declarator_;
    parser declarator_list_= seq( declarator, many( seq( comma_, declarator_list ) ) );
          *declarator_list = *declarator_list_;
    parser decl_specifiers = PLUS( type_specifier, sc_specifier,
                                   seq( type_specifier, sc_specifier ),
                                   seq( sc_specifier, type_specifier ) );
    parser declaration = seq( decl_specifiers, maybe( declarator_list ) );
    parser declaration_list = forward();
    parser declaration_list_= seq( declaration, maybe( seq( comma_, declaration_list ) ) );
          *declaration_list = *declaration_list_;
    parser data_def = zero();
    parser parameter_list = forward();
    parser parameter_list_= maybe( seq( expression, maybe( seq( comma_, parameter_list ) ) ) );
          *parameter_list = *parameter_list_;
    parser function_declarator = SEQ( declarator, lparen_, maybe( parameter_list ), rparen_ );
    parser function_statement = SEQ( lbrace_, maybe( declaration_list ), many( statement ), rbrace_ );
    parser function_body = seq( type_decl_list, function_statement );
    parser function_def = SEQ( maybe( type_specifier ), function_declarator, function_body );
    parser external_def = plus( function_def, data_def );
    parser program = some( external_def );

    p = program;
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

#define PRINT(__) printf( "%s =\n", #__ ), print( __ ), puts("")
#define PRINT_FLAT(__) printf( "%s =\n", #__ ), print_flat( __ ), puts("")
int test_parser(){
  char *source =
"int max(a, b, c)\n"
"int a, b, c;\n"
"{\n"
"      int m;\n"
"      m = (a>b)? a:b;\n"
"      return(m>c? m:c);"
"}\n"
"\t if(  2  ){\n\t   x = 5;\n\t   } int auto";
  object tokens = tokens_from_chars( chars_from_string( source ) );
  add_global_root( tokens );
  PRINT( take( 4, tokens ) );
  object program = tree_from_tokens( tokens );
  //PRINT( program );
  printf( "gc: %d\n", garbage_collect( program ) );
  PRINT_FLAT( x_( x_( program ) ) );
  PRINT( xs_( x_( program ) ) );
}

int main(){ test_parser(); }
