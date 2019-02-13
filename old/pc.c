#include <stdio.h>
#include <stdlib.h>


enum parser_type { TERM, ALT, SEQ };
typedef struct parser *parser;
struct parser {
    enum parser_type type;
    int c;
    parser a, b;
};


parser term( int c );
parser alt( parser a, parser b );
parser seq( parser a, parser b );
int parse( char *input, parser p );

parser
term( int c ){
    parser p = malloc( sizeof *p );
    p->type = TERM;
    p->c = c;
    return  p;
}

int parse_term( char *input, parser p ){
    printf( "%c?\n", p->c );
    return  *input == p->c;
}

parser
alt( parser a, parser b ){
    parser p = malloc( sizeof *p );
    p->type = ALT;
    p->a = a;
    p->b = b;
    return  p;
}

int parse_alt( char *input, parser p ){
    return  parse( input, p->a ) || parse( input, p->b );
}

parser
seq( parser a, parser b ){
    parser p = malloc( sizeof *p );
    p->type = SEQ;
    p->a = a;
    p->b = b;
    return  p;
}

int parse_seq( char *input, parser p ){
    return  parse( input, p->a ) ? parse( input + 1, p->b ) : 0;
}

int
parse( char *input, parser p ){
    switch( p->type ){
    case TERM:  return parse_term( input, p );
    case ALT:   return parse_alt( input, p );
    case SEQ:   return parse_seq( input, p );
    }
}

int
main( void ){
    parser p = seq( alt( term( 'H' ), term( 'h' ) ),
		    seq( term( 'e' ),
			 seq( term( 'l' ),
			      seq( term( 'l' ),
				   term( 'o' ) ) ) ) );
    printf("%d\n", parse( "hello", p ) );
    printf("%d\n", parse( "Hello", p ) );
    printf("%d\n", parse( "YaMaMa", p ) );
    return 0;
}
