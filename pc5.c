#include "pc5.h"

struct parser {
  handler this;
  parser a, b;
  int c, d;
  checker check;
  callback f;
  void *data;
};

define_new_function_for_( parser )
define_new_function_for_( result )
#define PARSER(...) new_parser( (struct parser){ __VA_ARGS__ } )
#define RESULT(...) new_result( (struct result){ __VA_ARGS__ } )
static result last_result( result r ){ return r->next ? last_result( r->next ) : r; }
static void amend_lengths( result r, int adjust ){
  if(  r  )  r->length_matched += adjust, amend_lengths( r->next, adjust );
}

result parse( parser p, char *input ){ return p->this( p, input ); }

static result p_fail   ( parser p, char *in ){ return NULL; }
static result p_succeed( parser p, char *in ){ return RESULT( NULL, 0 ); }
static result p_term   ( parser p, char *in ){ return *in == p->c  ? RESULT( NULL, 1 ): p_fail( p, in ); }
static result p_range  ( parser p, char *in ){ //printf("%c %c %c\n", p->c, *in, p->d );
  					       return p->c <= *in && *in <= p->d ?
    								     RESULT( NULL, 1 ): p_fail( p, in ); }
static result p_any    ( parser p, char *in ){ return *in          ? RESULT( NULL, 1 ): p_fail( p, in ); }
static result p_satisfy( parser p, char *in ){ return p->check(*in)? RESULT( NULL, 1 ): p_fail( p, in ); }
static result p_not( parser p, char *in ){ return parse( p->a, in ) ? p_fail( p, in )   : p_succeed( p, in ); }
static result p_and( parser p, char *in ){ return parse( p->a, in ) ? parse( p->b, in ) : p_fail( p, in ); }

static result p_alt( parser p, char *in ){
  result r = parse( p->a, in );
  return r ? last_result( r )->next = parse( p->b, in ), r : parse( p->b, in );
}
static result p_seq_next( result ra, parser p, char *in ){
  if(  ! ra  )  return p_fail( p, in );
  result rb = parse( p->b, in + ra->length_matched );
  if(  rb  ){
    amend_lengths( rb, ra->length_matched );
    return  last_result( rb )->next = p_seq_next( ra->next, p, in ),  rb;
  } else  return  p_seq_next( ra->next, p, in );
}
static result p_seq( parser p, char *in ){
  return p_seq_next( parse( p->a, in ), p, in );
}

static char *strndup(char*s,size_t n){char *r = malloc(n+1); return r? memcpy(r,s,n),r[n]=0,r: r;}
result p_call( parser p, char *in ){
  result r = parse( p->a, in );
  return  r ? p->f( p->data, strndup( in, r->length_matched ) ), r : r;
}

parser fails(){ return PARSER( .this = p_fail ); }
parser succeeds(){ return PARSER( .this = p_succeed ); }
parser empty(){ return PARSER(); }
parser term( int c ){ return PARSER( .this = p_term, .c = c ); }
parser range( int c, int d ){ return PARSER( .this = p_range, .c = c, .d = d ); }
parser any(){ return PARSER( .this = p_any ); }
parser not( parser a ){ return PARSER( .this = p_not, .a = a ); }
parser and( parser a, parser b ){ return PARSER( .this = p_and, .a = a, .b = b ); }
parser satisfies( checker check ){ return PARSER( .this = p_satisfy, .check = check ); }
parser alt2( parser a, parser b ){ return PARSER( .this = p_alt, .a = a, .b = b ); }
parser seq2( parser a, parser b ){ return PARSER( .this = p_seq, .a = a, .b = b ); }
parser maybe( parser a ){ return PARSER( .this = p_alt, .a = a, .b = succeeds() ); }

static void seq_alt_loop( parser a, parser *q, parser *r ){
  *q = PARSER( .this = p_seq, .a = a );
  *r = PARSER( .this = p_alt, .a = *q, .b = succeeds() );
  (*q)->b = *r;
}
parser many( parser a ){
  parser q,r;
  seq_alt_loop( a, &q, &r );
  return r;
}
parser some( parser a ){
  parser q,r;
  seq_alt_loop( a, &q, &r );
  return q;
}

parser seq( int n, ... ){
  va_list ap;
  va_start( ap, n );
  parser r = va_arg( ap, parser );
  while( --n ) r = seq2( r, va_arg( ap, parser ) );
  va_end( ap );
  return r;
}
parser alt( int n, ... ){
  va_list ap;
  va_start( ap, n );
  parser r = va_arg( ap, parser );
  while( --n ) r = alt2( r, va_arg( ap, parser ) );
  va_end( ap );
  return r;
}

parser cclass( char *str ){ return *str ? alt2( term( *str ), cclass( str + 1 ) ) : fails(); }
parser inv_cclass( char *str ){ return and( not( cclass( str ) ), any() ); }
parser string( char *str ){ return *str ? seq2( term( *str ), string( str + 1 ) ) : succeeds(); }
parser on( parser a, callback f, void *data ){
  return PARSER( .this = p_call, .a = a, .f = f, .data = data );
}


typedef struct plist *plist;
struct plist {
  plist next;
  parser it;
};
define_new_function_for_( plist )
#define PLIST(...) new_plist( (struct plist){ __VA_ARGS__ } )
static void ppush( plist *pl, parser p ){ *pl = PLIST( *pl, p ); }
static parser ppop( plist *pl ){ parser p; return  !*pl ? NULL : (p = (*pl)->it, *pl = (*pl)->next, p); }

static parser err( parser x ){ printf("unreachable error\n"); }
static void build_dot( void *p, char *s ){ plist *r = p; ppush( r, any() ); }
static void build_char( void *p, char *s ){ plist *r = p; ppush( r, term( *s ) ); }
static void build_range( void *p, char *s ){
  plist *r = p;
  parser x = ppop( r );
  //printf( "%c %c\n", x->c, *s );
  ppush( r, range( x->c, *s ) );
  free( x );
}
static void build_inv( void *p, char *s ){
  plist *r = p;
  parser x = ppop( r );
  ppush( r, and( not( x ), any() ) );
}
static void build_meta( void *p, char *s ){
  plist *r = p;
  parser x = ppop( r );
  ppush( r, (*s=='?'? maybe
	    :*s=='+'? some
	    :*s=='*'? many
	            : err  )( x ) );
}
static void build_facts( void *p, char *s ){
  plist *r = p;
  parser y = ppop( r ), x = ppop( r );
  ppush( r, SEQ( x, y ) );
}
static void build_terms( void *p, char *s ){
  plist *r = p;
  parser y = ppop( r ), x = ppop( r );
  ppush( r, ALT( x, y ) );
}

static parser build_re_parser( plist *r ){
  parser  Dot  = term('.');
  parser  Meta = cclass("?+*");
  parser  Or   = term('|');
  parser  Char = inv_cclass("^?+*.|()[]");
  parser  BeginChar = Char;
  parser  EndChar = Char;
  parser  CRange = SEQ( on( BeginChar, build_char, r ),
			maybe( SEQ( term('-'), on( EndChar, build_range, r ) ) ) );
  parser  CClass = SEQ( CRange, many( on( CRange, build_terms, r ) ) );
  parser  Expr = empty();
  parser  Atom = ALT( on( Dot, build_dot, r ),
		      SEQ( term('('), Expr, term(')') ),
		      SEQ( term('['), term('^'), on( CClass, build_inv, r ), term(']') ),
		      SEQ( term('['), CClass, term(']') ),
		      on( Char, build_char, r ) );
  parser  Fact = SEQ( Atom, maybe( on( Meta, build_meta, r ) ) );
  parser  Term = SEQ( Fact, many( on( Fact, build_facts, r ) ) );
  parser  Expr_ = SEQ( Term, many( SEQ( Or, on( Term, build_terms, r ) ) ) );
  return  *Expr = *Expr_, free( Expr_ ), Expr;
}

parser regex( char *re ){
  static parser p = NULL;
  static plist r;
  if(  !p  ) p = build_re_parser( &r );
  return  r = NULL,  parse( p, re ) ? ppop( &r ) : NULL;
}
