#include <stdio.h>
#include <stdlib.h>
#include "pc4re.h"

typedef struct plist *plist;
struct plist {
  parser it;
  plist next;
};
define_new_function_for_( plist )

void ppush( plist *pl, parser p ){ *pl = new_plist( (struct plist){ p, *pl } ); }
parser ppop( plist *pl ){ parser p; return  !*pl ? NULL : (p = (*pl)->it, *pl = (*pl)->next, p); }

void build_dot( void *p, char *s ){ plist *r = p;  ppush( r, any() ); }
void build_char( void *p, char *s ){ plist *r = p;  ppush( r, term( *s ) ); }
void build_meta( void *p, char *s){ plist *r = p;  parser x = ppop( r );
  ppush( r, (*s=='?'? maybe :*s=='+'? some :*s=='*'? many :err)( x ) );
}
void build_factors( void *p, char *s){
  plist *r = p;
  parser y = ppop( r );
  parser x = ppop( r );
  ppush( r , sequence( x, y ) );
}
void build_terms( void *p, char *s){
  plist *r = p;
  parser y = ppop( r );
  parser x = ppop( r );
  ppush( r, alternate( x, y ) );
}

parser regex( char *re ){
  static parser p = NULL;
  plist r = NULL;
  if(  !p  ){
    parser dot       = term('.');
    parser meta = char_class("?+*");
    parser or_  = term('|');
    parser character = any();
    parser atom = alternaten(2, (parser[]){
	  action( dot, build_dot, &r),
	  action( character, build_char, &r)
	});
    parser factor = sequence( atom, maybe( action( meta, build_meta, &r ) ) );
    parser term = sequence( factor, many( action( factor, build_factors, &r ) ) );
    parser expression = sequence( term, many( sequence( or_, action( term, build_terms, &r ) ) ) );
    p = expression;
  }
  return  parse( p, re )? r->it :0;
}
#include <stdio.h>
#include <stdlib.h>
#include "pc4re.h"

typedef struct plist *plist;
struct plist {
  parser it;
  plist next;
};
define_new_function_for_( plist )

int noprintf(char *f,...){return 0;}

void ppush( plist *pl, parser p ){ *pl = new_plist( (struct plist){ p, *pl } ); }
parser ppop( plist *pl ){ parser p; return  !*pl ? NULL : (p = (*pl)->it, *pl = (*pl)->next, p); }

void build_dot( void *p, char *s ){ plist *r = p; noprintf(".\n"); ppush( r, any() ); }
void build_char( void *p, char *s ){ plist *r = p; noprintf("%c\n", *s); ppush( r, term( *s ) ); }
void build_meta( void *p, char *s){ plist *r = p;
  noprintf("%c\n", *s);
  parser x = ppop( r );
  switch(*s){
  case '?': ppush( r, maybe( x ) ); break;
  case '+': ppush( r, some( x ) ); break;
  case '*': ppush( r, many( x ) ); break;
  }
}
void build_factors( void *p, char *s){
  plist *r = p;
  parser y = ppop( r );
  parser x = ppop( r );
  ppush( r , sequence( x, y ) );
}
void build_terms( void *p, char *s){
  plist *r = p;
  parser y = ppop( r );
  parser x = ppop( r );
  ppush( r, alternate( x, y ) );
}

parser regex( char *re ){
  static parser p = NULL;
  plist r = NULL;
  if(  !p  ){
    parser dot       = term('.');
    parser meta = char_class("?+*");
    parser or_  = term('|');
    parser character = any();
    parser atom = alternaten(2, (parser[]){
	  action( dot, build_dot, &r),
	  action( character, build_char, &r)
	});
    parser factor = sequence( atom, maybe( action( meta, build_meta, &r ) ) );
    parser term = sequence( factor, many( action( factor, build_factors, &r ) ) );
    parser expression = sequence( term, many( sequence( or_, action( term, build_terms, &r ) ) ) );
    p = expression;
  }
  return  parse( p, re ) ? noprintf("x\n"), r->it : 0;
}
