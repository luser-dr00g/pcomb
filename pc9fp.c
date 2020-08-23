#include <stdarg.h>
#include <string.h>
#include "pc9fp.h"
#include "pc9objpriv.h"

boolean
eq( object a, object b ){
  return (
           !valid( a ) && !valid( b )  ? 1  :
           !valid( a ) || !valid( b )  ? 0  :
           a->t != b->t                ? 0  :
           a->t == SYMBOL              ? a->Symbol.symbol == b->Symbol.symbol  :
           !memcmp( a, b, sizeof *a )  ? 1  : 0
         )  ? T_  : NIL_;
}


// Association Lists

list
env( list tail, int n, ... ){
  va_list v;
  va_start( v, n );
  list r = tail;
  while( n-- ){
    object a = va_arg( v, object );
    object b = va_arg( v, object );
    r = cons( cons( a, b ), r );
  }
  va_end( v );
  return  r;
}

object
assoc( object a, list b ){
  return  !valid( b )                      ? NIL_            :
          valid( eq( a, x_( x_( b ) ) ) )  ? xs_( x_( b ) )  :
                                             assoc( a, xs_( b ) );
}

object
assoc_symbol( int sym, list b ){
  return  assoc( (union uobject[]){ {.Symbol = {SYMBOL, sym,"",0}} }, b );
}


// Lists

list
copy( list a ){
  return  !valid( a )   ? NIL_                                       :
          a->t == LIST  ? cons( copy( x_( a ) ), copy( xs_( a ) ) )  :
          		  a;
}

static list
force_append( object v ){
  list a = assoc_symbol( APPEND_A, v );
  list b = assoc_symbol( APPEND_B, v );
  *a = *force_( a );
  return  append( a, b );
}
list
append( list a, list b ){
  return  !valid( a )  ?
            b
          : a->t == SUSPENSION  ?
            Suspension( env( 0, 2, Symbol(APPEND_A), a, Symbol(APPEND_B), b ),
                        force_append )
          : a->t == LIST  ?
            cons( x_( a ), append( xs_( a ), b ) )
          : cons( a, b );
}


// Functions over lists

static object
force_apply( object v ){
  oper f = assoc_symbol( APPLY_F, v );
  object o = assoc_symbol( APPLY_X, v );
  *o = *force_( o );
  return  valid( o )  ? f->Operator.f( f->Operator.v, o )  : NIL_;
}
object
apply( oper f, object o ){
  return  f->t == OPERATOR  ? 
            valid( o )  ?
              o->t == SUSPENSION  ?
                Suspension( env( 0, 2, Symbol(APPLY_F), f, Symbol(APPLY_X), o ),
                            force_apply )
              : f->Operator.f( f->Operator.v, o )
            : f->Operator.f( f->Operator.v, o ) //for using(maybe(),...)
          : NIL_;
  //return  f->t == OPERATOR  ? f->Operator.f( f->Operator.v, o )  : NIL_;
}


static list
force_map( object v ){
  oper f = assoc_symbol( MAP_F, v );
  list o = assoc_symbol( MAP_X, v );
  if(  !valid( o )  ) return  NIL_;
  *o = *force_( o );
  return  valid( o )  ?
            cons( apply( f, x_( o ) ), map( f, xs_( o ) ) )
          : NIL_;
}
list
map( oper f, list o ){
  return  valid( o )  ?
            o->t == SUSPENSION  ?
              Suspension( env( 0,2,Symbol(MAP_F),f,Symbol(MAP_X),o ), force_map )
            : cons( apply( f, x_( o ) ),
	      Suspension( env( 0,2,Symbol(MAP_F),f,Symbol(MAP_X),xs_(o) ), force_map ) )
	  : NIL_;
  //return  valid( o )? cons( apply( f, x_( o ) ), map( f, xs_( o ) ) ): NIL_;
}


static list
force_join( object v ){
  list o = assoc_symbol( JOIN_X, v );
  *o = *force_( o );
  return  append( x_( take( 1, o ) ), join( xs_( o ) ) );
}
list
join( list o ){
  return  valid( o )  ? 
            o->t == SUSPENSION  ?
              Suspension( env( 0, 1, Symbol(JOIN_X), o ), force_join )
            : append( x_( o ),
	        Suspension( env( 0,1,Symbol(JOIN_X),xs_( o ) ), force_join ) )
          : NIL_;
  //return  valid( o )  ? append( x_( o ), join( xs_( o ) ) )  : NIL_;
}


// Folds

static object
do_collapse( fBinOper *f, object a, object b ){
  return  valid( b )  ? f( a, b )  : a;
}

object
collapse( fBinOper *f, list o ){
  return  valid( o )  ?
            o->t == LIST  ?
              do_collapse( f, collapse( f, x_( o ) ), collapse( f, xs_( o ) ) )
	    : o
	  : NIL_;
}

// f( po[0], f( po[1], ... f( po[n-2], po[n-1] ) ... ) )
object
reduce( fBinOper *f, int n, object *po ){
  return  n==1  ? *po  : f( *po, reduce( f, n-1, po+1 ) );
}

// f( f( ... f( f( po[0], po[1] ), po[2] ) ... , po[n-2] ), po[n-1] )
object
rreduce( fBinOper *f, int n, object *po ){
  return  n==1  ? *po  : f( reduce( f, n-1, po ), po[ n-1 ] );
}


static list
force_chars_with_positions( list v ){
  list input = assoc_symbol( INPUT, v );
  if(  !valid( input )  ) return  NIL_;
  object pos = assoc_symbol( POS, v );
  object c = x_( take( 1, input ) );
  if(  c->t == SYMBOL && c->Symbol.symbol == EOF  ) return one( c );
  *input = *xs_( input );
  if(  c->Int.i == '\n'  )
      x_( pos )->Int.i = 0, xs_( pos )->Int.i += 1;
  else
      x_( pos )->Int.i += 1;
  return  cons( cons( c, copy( pos ) ), 
                Suspension( v, force_chars_with_positions ) );
}

list
chars_with_positions( list o ){
  return  o  ? Suspension(
                   env( 0, 2, Symbol(INPUT), o, Symbol(POS), cons( Int(0), Int(0) ) ), 
                   force_chars_with_positions )
             : NIL_;
}


// Pattern Matching
list matchpat( list v, list pat, list a );

list
match_list( list v, list pat, list a ){
  //PRINT(pat);
  //PRINT(a);
  if(  xs_( pat )->t == OPERATOR  ){
    list av = matchpat( v, x_(pat), a );
    if(  valid( av )  ){
      return  matchpat( av, xs_(pat), a );
    }
  } else {
    list av = matchpat( v, x_(pat), x_(a) );
    //PRINT(av);
    if(  valid( av )  ){
      list bv = matchpat( av==T_  ? v : av, xs_(pat), xs_(a) );
      //PRINT(bv);
      return  bv;
    }
  }
  return  0;
}

list
match_symbol( list v, list pat, list a ){
  //PRINT(pat);
  //PRINT(a);
  if(  pat->Symbol.data==T_  ){
    return  eq( pat, a );
  } else {
    return  env( v, 1, pat, a );
  }
}

list
match_operator( list v, list pat, list a ){
  return  pat->Operator.f( pat->Operator.v, v );
}

// (t_id x) => print x
// ( ( <t_id:T> <x:NIL> ) :oper{print x} )
list
matchpat( list v, list pat, list a ){
  if(  !pat  || !a  ) return  0;
  switch(  pat->t  ){
  case LIST:     return  match_list( v, pat, a );
  case SYMBOL:   return  match_symbol( v, pat, a );
  case OPERATOR: return  match_operator( v, pat, a );
  default:       return  eq( pat, a );
  }
}

list
match( list pats, list a ){
  if(  !valid(pats)  ) return  0;
  list av = matchpat( 0, x_(pats), a );
  return  valid( av )  ? av  : match( xs_(pats), a );
}

list
unembed_symbols( list a ){
  if(  !a  ) return  a;
  switch(  a->t  ){
  case LIST: return  cons( unembed_symbols( a->List.a ),
                           unembed_symbols( a->List.b ) );
  case SYMBOL: return
      a->Symbol.data  ? cons( Symbol_( a->Symbol.symbol, a->Symbol.pname, 0 ),
                              unembed_symbols( a->Symbol.data ) )
                      : Symbol_( a->Symbol.symbol, a->Symbol.pname, 0 );
  }
  return  a;
}
