#include <stdio.h>
#include "pc4re.h"
#include "pc4priv.h"
#include "pc4eg.h"

define_new_function_for_( tree )

void tpush( tree *root, tree node ){
  node->next = *root;
  *root = node;
}
tree tpop( tree *root ){
  tree node = *root;
  *root = node->next;
  node->next = NULL;
  return node;
}
tree last_next( tree t ){ return  t->next  ?  last_next( t->next )  :  t; }
tree last_child( tree t ){ return  last_next( t->first ); }


void make_id( void *p, char *s ){
  tree *t = p;
  tpush( t, new_tree( (struct tree){ (token){ ID, .id = s }, NULL, NULL } ) );
}

void make_int( void *p, char *s ){
  tree *t = p;
  tpush( t, new_tree( (struct tree){ (token){ INT, .i = atoi( s ) }, NULL, NULL } ) );
}

void make_op( void *p, char *s ){
  tree *t = p;
  tree y = tpop( t );
  tree x = tpop( t );
  x->next = y;
  tpush( t, new_tree( (struct tree){ (token){ OP, .op = *s }, NULL, x } ) );
}

void make_expr_stmt( void *p, char *s ){
  tree *t = p;
  tree x = tpop( t );
  tpush( t, new_tree( (struct tree){ (token){ STMT }, NULL, x } ) );
}

void make_empty_stmt( void *p, char *s ){
  tree *t = p;
  tpush( t, new_tree( (struct tree){ (token){ STMT }, NULL, NULL } ) );
}

void append_stmt( void *p, char *s ){
  tree *t = p;
  tree y = tpop( t );
  tree x = tpop( t );
  last_child( x )->next = y;
  tpush( t, x );
}

parser eat_ws( parser p ){
  return  sequence( many( char_class( " \t\n" ) ), p );
}

parser tok(char *s) {
  return  string( s );
}

parser build_parser( void *r ){
  parser Integer = some( char_class( "0123456789" ) );
  parser String = some( char_class( "abcdefghijklmnopqrstuvwxyz" ) );
  parser Id = String;
  parser Expression = empty();
  parser Paren_Expression = sequencen(3, (parser[]){ tok("("), Expression, tok(")") });
  parser Term = alternaten(3, (parser[]){
      eat_ws( action( Id,              make_id, r ) ),
	eat_ws( action( Integer,         make_int, r ) ),
		Paren_Expression
      });
  parser Sum = sequence( Term,
			 many( eat_ws( action( sequence( alternate( tok("+"), tok("-") ), Term ), make_op, r ) ) ) );
  parser Test = sequence( Sum, maybe( eat_ws( action( sequence( tok("<"), Sum ), make_op, r ) ) ) );
  *Expression = *alternate( Test, sequence( Id, eat_ws( action( sequence( tok("="), Expression ), make_op, r ) ) ) );
  parser Statement = empty();
  *Statement = *alternaten(3, (parser[]){
        sequencen(4, (parser[]){
	        tok("{"), maybe( Statement ), many( action( Statement, append_stmt, r ) ), tok("}")
	    } ),
	  action( sequence( Expression, eat_ws( tok(";") ) ), make_expr_stmt, r ),
	  eat_ws( action( tok(";"), make_empty_stmt, r ) )
    });
  parser Program = sequence( Statement, many( action( Statement, append_stmt, r ) ) );
  return  Program;
}

tree parse_program(char *s){
  static parser p = NULL;
  static tree r = NULL;
  if(  !p  ) p = build_parser( &r );
  return  r = NULL,  parse( p, s ),  r;
}


void format_expr( tree t );

void format_id( tree t ){
  printf( "%s", t->value.id );
}

void format_int(tree t ){
  printf( "%d", t->value.i );
}

void format_equal( tree t ){
  format_id( t->first );
  printf("=");
  format_expr( t->first->next );
}
void format_plus( tree t ){
}
void format_less( tree t ){
}
void format_minus( tree t ){
}

void format_op( tree t ){
  switch( t->value.op ){
  case '=': format_equal( t ); break;
  case '+': format_plus( t ); break;
  case '<': format_less( t ); break;
  case '-': format_minus( t ); break;
  }
}

void format_expr( tree t ){
  switch( t->value.type ){
  case ID: format_id( t ); break;
  case INT: format_int( t ); break;
  case OP: format_op( t ); break;
  }
}

void format_stmt( tree t ){
  switch( t->value.type ){
  case STMT: t->first? format_expr( t->first ): 0; break;
  }
  printf(";");
}

void format_stmts( tree t ){
  t ? format_stmt( t ), format_stmts( t->next ) : 0;
}

void format_program( tree t ){
  format_stmts( t );
  printf("\n");
}
