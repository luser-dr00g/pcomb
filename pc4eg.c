#include "pc4re.h"
#include "pc4priv.h"

typedef struct token token;
struct token {
  enum { ID, INT, OP, STMT } type;
  char *id;
  int    i;
  char  op;
};

typedef struct tree *tree;
struct tree {
  token value;
  tree next;
  tree first;
};
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

parser build_parser( void *r ){
    parser Integer = some( char_class( "0123456789" ) );
    parser String = some( char_class( "abcdefghijklmnopqrstuvwxyz" ) );
    parser Id = String;
    parser Expression = empty();
    parser Paren_Expression = sequencen(3, (parser[]){ term('('), Expression, term(')') });
    parser Term = alternaten(3, (parser[]){
	  action( Id,              make_id, &r ),
	  action( Integer,         make_int, &r ),
	          Paren_Expression
	});
    parser Sum = sequence( Term,
			  many( action( sequence( alternate( term('+'), term('-') ), Term ), make_op, &r ) ) );
    parser Test = sequence( Sum, maybe( action( sequence( term('<'), Sum ), make_op, &r ) ) );
    *Expression = *alternate( Test, sequence( Id, action( sequence( term('='), Expression ), make_op, &r ) ) );
    parser Statement = empty();
    *Statement = *alternaten(3, (parser[]){
	  sequencen(4, (parser[]){
	    term('{'), maybe( Statement ), many( action( Statement, append_stmt, &r ) ), term('}') } ),
	  action( sequence( Expression, term(';') ), make_expr_stmt, &r ),
	  action( term(';'), make_empty_stmt, &r )
	});
    parser Program = sequence( Statement, many( action( Statement, append_stmt, &r ) ) );
    return  Program;
}

tree parse_program(char *s){
  static parser p = NULL;
  static tree r = NULL;
  if(  !p  ) p = build_parser( &r );
  return  r = NULL, parse(p, s), r;
}
