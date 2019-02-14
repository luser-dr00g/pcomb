#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pc4.h"

result parse_fail( parser p, char *input );
result parse_succeed( parser p, char *input );
result parse_term( parser p, char *input );
result parse_any( parser p, char *input );
result parse_alternate( parser p, char *input );
result parse_sequence( parser p, char *input );

result last_result( result r );
static void amend_lengths( result r, int length );
static result parse_sequence_next( result r, parser p, char *input );


Types( define_new_function_for_ )


/* entry point to the parser tree */
result parse( parser p, char *input ){
  return  p->test( p, input );
}



/* handlers for the nodes */

/* * leaf nodes */

result parse_fail( parser p, char *input ){
  return  NULL;
}

result parse_succeed( parser p, char *input ){ 
  return  new_result( (struct result){ .next = NULL, .length_matched = 0 } );
}

result parse_term( parser p, char *input ){ 
  return  *input == p->c ? new_result( (struct result){ .next = NULL, .length_matched = 1 } )  :  NULL; 
} 

result parse_any( parser p, char *input ){
  return  *input ? new_result( (struct result){ .next = NULL, .length_matched = 1 } ) : NULL;
}


/* * branch nodes */
result parse_not( parser p, char *input ){
  result r = parse( p->a, input );
  if(  r  )
    return NULL;
  else
    return new_result( (struct result){ .next = NULL, .length_matched = 0 } );
}

result parse_and( parser p, char *input ){
  result r = parse( p->a, input );
  if(  r  )
    return  parse( p->b, input );
  else
    return  NULL;
}

const int short_circuit_alternate = 0;

/* append to the results of the left branch
   the results of the right branch */
result parse_alternate( parser p, char *input ){ 
  result r = parse( p->a, input ); 
  if(  r  )
    return  (short_circuit_alternate ? 0 : (last_result( r )->next = parse( p->b, input ))),
            r; 
  else
    return  parse( p->b, input ); 
}

/* call left branch, process any results */
result parse_sequence( parser p, char *input ){ 
  return  parse_sequence_next( parse( p->a, input ), p, input ); 
} 

/* for each result of left branch, offset input by length matched,
   and try right branch, adding the left length to any results of 
   right branch */
static result parse_sequence_next( result ra, parser p, char *input ){ 
  if(  ! ra  )  return  NULL; 
  result rb = parse( p->b, input + ra->length_matched ); 
  if(  rb  ){ 
    amend_lengths( rb, ra->length_matched ); 
    return  last_result( rb )->next = parse_sequence_next( ra->next, p, input ), rb; 
  } else  return  parse_sequence_next( ra->next, p, input ); 
} 


result last_result( result r ){
  return  r->next  ?  last_result( r->next )  :  r;
}

static void amend_lengths( result r, int length ){ 
  if(  r  )  r->length_matched += length; 
  if(  r->next  )  amend_lengths( r->next, length ); 
} 


/* constructors */

/* *  "axioms" */
parser fails(){     return  new_parser( (struct parser){ .test = parse_fail    } ); }
parser succeeds(){  return  new_parser( (struct parser){ .test = parse_succeed } ); }

parser empty(){     return  new_parser( (struct parser){0} ); }
parser term( int c ){
  return  new_parser( (struct parser){ .test = parse_term, .c = c } );
}

parser any(){
  return  new_parser( (struct parser){ .test = parse_any } );
}

parser not( parser a ){
  return  new_parser( (struct parser){ .test = parse_not, .a = a } );
}

parser and( parser a, parser b ){
  return  new_parser( (struct parser){ .test = parse_and, .a = a, .b = b } );
}

parser alternate( parser a, parser b ){
  return  new_parser( (struct parser){ .test = parse_alternate, .a = a, .b = b } );
}

parser sequence( parser a, parser b ){
  return  new_parser( (struct parser){ .test = parse_sequence, .a = a, .b = b } );
}

/* *  "theorems" */
parser char_class( char *str ){
  return  *str  ?  alternate( term( *str ), char_class( str + 1 ) )  :  fails();
}

parser inverse_char_class( char *str ){
  return  and( not( char_class( str ) ), any() );
}

parser string( char *str ){
  return  *str  ?  sequence( term( *str ), string( str + 1 ) )  :  succeeds();
}

parser maybe( parser a ){
  return  new_parser( (struct parser){ .test = parse_alternate, .a = a, .b = succeeds() } );
}

parser many( parser a ){
  parser q = new_parser( (struct parser){ .test = parse_sequence, .a = a } );
  parser r = new_parser( (struct parser){ .test = parse_alternate, .a = q, .b = succeeds() } );
  q->b = r;
  return r;
}

parser some( parser a ){
  parser q = new_parser( (struct parser){ .test = parse_sequence, .a = a } );
  parser r = new_parser( (struct parser){ .test = parse_alternate, .a = q, .b = succeeds() } );
  q->b = r;
  return q;
}

parser sequencen( int n, parser *pp ){
  return  n  ?  sequence( *pp, sequencen( n-1, pp+1 ) )
    	     :  succeeds();
}


parser alternaten( int n, parser *pp ){
  return  n  ?  alternate( *pp, alternaten( n-1, pp+1 ) )
    	     :  fails();
}

//char *strndup(char*s,size_t n){char *r = malloc(n+1); if (r) memcpy(r,s,n),r[n]=0; return r;}

result parse_action( parser p, char *input ){
  result r = parse( p->a, input );
  if (r) p->f( p->payload, strndup( input, r->length_matched ) );
  return  r;
}

parser action( parser pa, callback f, void *payload ){
  return  new_parser( (struct parser){ .test = parse_action, .a = pa, .f = f, .payload = payload } );
}

