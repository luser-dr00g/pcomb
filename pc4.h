typedef struct parser *parser;
typedef struct result *result;
typedef result (*handler)( parser, char *);
typedef void (*callback)( void *payload, char *match );

struct parser { 
  handler test; 
  parser a, b; 
  int c; 
  callback f;
  void *payload;
}; 

struct result { 
  result next; 
  int length_matched; 
}; 

#define Types(_) \
  _(parser)      \
  _(result)
#define declare_new_function_for_(type) \
  type new_##type ( struct type input );
Types( declare_new_function_for_ )

#define define_new_function_for_(type)                            \
  type new_##type ( struct type input ){                   \
    type local = calloc( sizeof *local, 1 );               \
    return  local  ?  ( *local = input ), local  :  local; \
  }


// Parsing actions return result lists
result parse( parser p, char *input );

// Constructors build parsers
parser fails();
parser succeeds();
parser term( int c );
parser any();
parser alternate( parser a, parser b );
parser sequence( parser a, parser b );
parser char_class( char *str );
parser string( char *str );
parser maybe( parser a );
parser many( parser a );
parser some( parser a );
parser sequencen( int n, parser *pp );
parser alternaten( int n, parser *pp );

// Hook a callback into the parser tree
parser action( parser a, callback f, void *payload );
