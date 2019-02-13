#include <stdio.h>
#include <stdlib.h>

typedef struct parser *parser;
typedef int (*parser_handler)( parser p, char *input );

struct parser {
    parser_handler test;
    parser a, b;
    int c;
};

int debug_parse_term = 1;


int parse( parser p, char *input ){    return p->test( p, input ); }

int parse_fail   ( parser p, char *input ){    return  -1; }
int parse_succeed( parser p, char *input ){    return  0; }

int parse_term( parser p, char *input ){
    return  debug_parse_term && printf( "%c%c? ", p->c, *input ),
            *input == p->c  ?  1  :  -1;
}
int parse_alternate ( parser p, char *input ){
    int i;
    return  ( i = parse( p->a, input ) ) >= 0  ?  i
                                               :  parse( p->b, input );
}
int parse_sequence ( parser p, char *input ){
  int i,j;
    return  ( i = parse( p->a, input     ) ) >= 0  ?
            ( j = parse( p->b, input + i ) ) >= 0  ?  i + j
      						   :  -1
						   :  -1;
}


parser new_parser( struct parser ps ){
    parser p = malloc( sizeof *p );
    return  p  ?  ( *p = ps ), p  :  p;
}

parser fails   (){    return  new_parser( (struct parser){ .test = parse_fail } ); }
parser succeeds(){    return  new_parser( (struct parser){ .test = parse_succeed } ); }

parser term( int c ){
    return  new_parser( (struct parser){ .test = parse_term, .c = c } );
}
parser alternate2( parser a, parser b ){
    return  new_parser( (struct parser){ .test = parse_alternate, .a = a, .b = b } );
}
parser sequence2 ( parser a, parser b ){
    return  new_parser( (struct parser){ .test = parse_sequence, .a = a, .b = b } );
}

parser many( parser a ){
    parser q = new_parser( (struct parser){ .test = parse_sequence,  .a = a } );
    parser r = new_parser( (struct parser){ .test = parse_alternate, .a = q, .b = succeeds() } );
    q->b = r;
    return r;
}

parser some( parser a ){
    parser q = new_parser( (struct parser){ .test = parse_sequence,  .a = a } );
    parser r = new_parser( (struct parser){ .test = parse_alternate, .a = q, .b = succeeds() } );
    q->b = r;
    return q;
}

parser n_or_more( int n, parser a ){
    return  n == 0  ?  many( a )  :
      	    n == 1  ?  some( a )  :
                       sequence2( a, n_or_more( n - 1, a ) );
}

parser char_class( char *str ){
    return  *str  ?  alternate2( term( *str ), char_class( str + 1 ) )
                  :  fails();
}

parser string( char *str ){
    return  *str  ?  sequence2( term(*str), string( str + 1 ) )
                  :  succeeds();
}

parser sequencen( int n, parser *pp ){
    return  n  ?  sequence2( *pp, sequencen( n - 1, pp + 1 ) )
               :  succeeds();
}

parser alternaten( int n, parser *pp){
    return  n  ?  alternate2( *pp, alternaten( n - 1, pp + 1 ) )
               :  fails();
}


int test( parser p, char *str ){
    printf( "%s:", str );
    fflush(0);
    return printf( "%d\n", parse( p, str ) );
}

int main( void ){
  parser p = sequencen( 7, (parser[]){
      char_class( "Hhj" ), string( "ello " ),
	char_class( "Wwm" ), string( "o" ), char_class( "ru" ), string( "ld" ),
	char_class("!.") });
    
    test( p, "hello World." );
    test( p, "jello mould!" );
    test( p, "YaMaMa" );

    parser q = many( term( 'x' ) );
    test( q, "y" );
    test( q, "xy" );
    test( q, "xxy" );
    
    parser r = some( term( 'x' ) );
    test( r, "y" );
    test( r, "xy" );
    test( r, "xxy" );

    parser s = n_or_more( 3, term( 'a' ) );
    test( s, "aa" );
    test( s, "aaa" );
    test( s, "aaaa" );
    test( s, "aaaaa" );

    parser number = some( char_class("0123456789") );
    test( number, "0 f" );
    test( number, "f 0" );
    test( number, "123f" );
    test( number, "9999aaa" );

    number = sequence2( many( term( ' ' ) ), number );
    test( number, " 0 f" );
    test( number, "  f 0" );
    test( number, "   123f" );
    test( number, "    9999aaa" );

    parser t = sequence2(some(term('x')), string("xy"));
    test(t, "xxxxy"); 

    return 0;
}
