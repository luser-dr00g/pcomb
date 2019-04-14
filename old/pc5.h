#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ppnarg.h"
typedef struct parser *parser;
typedef struct result *result;
typedef result (*handler)( parser, char * );
typedef void (*callback)( void *userdata, char *match );
typedef int (*checker)( int c );

struct result {
  result next;
  int length_matched;
};

static int print_results( result r ){
  return r ? printf("%d", r->length_matched), r->next ? printf(", "), print_results( r->next ) : 0 : 0;
}
static int max_int( int a, int b ){return a > b ? a : b;}
static int max_result( result r ){
  return r ? r->next ? max_int( r->length_matched, r->next->length_matched ) : r->length_matched : -1;
}


#define declare_new_function_for_( type ) type new_##type( struct type input );
#define define_new_function_for_( type )  type new_##type( struct type input ){              \
					    type local = calloc( sizeof *local, 1 );         \
					    return  local ? ( *local = input), local : NULL; \
					  }

parser fails();
parser succeeds();
parser empty();
parser term( int c );
parser any();			   // .
parser not( parser a );
parser and( parser a, parser b );
parser satisfies( checker check );
parser maybe( parser a );          // ?
parser many( parser a );           // *
parser some( parser a );           // +
parser alt2( parser a, parser b );
parser alt( int n, ... );
#define ALT( ... ) alt( PP_NARG(__VA_ARGS__), __VA_ARGS__ )
parser seq2( parser a, parser b );
parser seq( int n, ... );
#define SEQ( ... ) seq( PP_NARG(__VA_ARGS__), __VA_ARGS__ )
parser cclass( char *s );
parser inv_cclass( char *s );
parser string( char *s );
parser on( parser a, callback f, void *data );

parser regex( char *re );

result parse( parser p, char *input );

/*
TODO
 memory responsibility
 abstract away char* from parse()  ... utf8?
 error reporting
 unit type?
 */
