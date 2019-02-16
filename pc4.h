#define PC4_H_
typedef struct parser *parser;
typedef struct result *result;
typedef void (*callback)( void *payload, char *match );

struct result { 
  result next; 
  int length_matched; 
}; 

// Parsing actions return result lists
result parse( parser p, char *input );

// Constructors build parsers
parser fails();
parser succeeds();
parser empty();
parser term( int c );
parser any();
parser not( parser a );
parser and( parser a, parser b );
parser alternate( parser a, parser b );
parser sequence( parser a, parser b );
parser char_class( char *str );
parser inverse_char_class( char *str );
parser string( char *str );
parser maybe( parser a );
parser many( parser a );
parser some( parser a );
parser sequencen( int n, parser *pp );
parser alternaten( int n, parser *pp );

// Hook a callback into the parser tree
parser action( parser a, callback f, void *payload );
