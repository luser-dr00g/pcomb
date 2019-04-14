#define POINTER_TO *
typedef union   object_  POINTER_TO object;
typedef object  parser;
typedef object  list;
typedef object  operator;
typedef object  predicate;
typedef object  boolean;

list string_input( void * );
list parse( parser, list );
void print( object );

parser result( object );
parser zero( void );
parser item( void );

parser bind( parser, operator );
parser plus( parser, parser );
parser sat( predicate );

parser lit( object );
parser Char( int );
parser anyof( char * );
parser noneof( char * );

parser seq( parser, parser );
parser xthen( parser, parser );
parser thenx( parser, parser );
parser into( parser, object, parser );

parser maybe( parser );
parser many( parser );
parser some( parser );
parser trim( parser );

parser regex( char * );

int pc_main();
