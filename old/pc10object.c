#include "pc10object.h"

struct memory memory;

object
Int( int i ){
  return  (object){ .Int = { INT, i } };
}

boolean
Bool( int b ){
  return  b  ? T_  : NIL_;
}

object
Void( void *ptr ){
  return  (object){ .Void = { VOID, ptr } };
}


int
addtocache( struct bank *bank, void *ptr ){
  if(  bank->used == bank->max  ){
    bank->max = bank->max * 7 + 11;
    void *tmp = realloc( bank->table, bank->max * sizeof(void*) );
    if(  !tmp  ) return -1;
    bank->table = tmp;
  }
  int index = (bank->used)++;
  (bank->table)[ index ] = ptr;
  return  index;
}

object
cache( tag t, void *ptr ){
  if(  t < FIRST_INDEXED_TYPE || t > LAST_INDEXED_TYPE  ) return  NIL_;
  int index = addtocache( & memory.bank[ t - FIRST_INDEXED_TYPE ], ptr );
  if(  index < 0  ) return  NIL_;
  return  (object){ .Ref = { t, index } };
}

void *
allocate( tag t, size_t sz ){
  return  malloc( sz );
}


list
cons( object a, object b ){
  struct list *ptr = allocate( LIST, sizeof *ptr );
  if(  !ptr  ) return  NIL_;
  *ptr = (struct list){ a, b };
  return  cache( LIST, ptr );
}

list
one( object it ){
  return  cons( it, NIL_ );
}

object
Suspension( object env, fSuspension *f ){
  struct suspension *ptr = allocate( SUSPENSION, sizeof *ptr );
  if(  !ptr  ) return  NIL_;
  *ptr = (struct suspension){ env, f };
  return  cache( SUSPENSION, ptr );
}

object
Parser( object env, fParser *f ){
  struct parser *ptr = allocate( PARSER, sizeof *ptr );
  if(  !ptr  ) return  NIL_;
  *ptr = (struct parser){ env, f };
  return  cache( PARSER, ptr );
}

object
Operator( object env, fOperator *f ){
  struct operator *ptr = allocate( OPERATOR, sizeof *ptr );
  if(  !ptr  ) return  NIL_;
  *ptr = (struct operator){ env, f };
  return  cache( OPERATOR, ptr );
}

object
Symbol_( int code, char *printname, object data ){
  struct symbol *ptr = allocate( SYMBOL, sizeof *ptr );
  if(  !ptr  ) return  NIL_;
  *ptr = (struct symbol){ code, printname, data };
  return  cache( SYMBOL, ptr );
}

object
String( char *str, int disposable ){
  struct string *ptr = allocate( STRING, sizeof *ptr );
  if(  !ptr  ) return  NIL_;
  *ptr = (struct string){ str, disposable };
  return  cache( STRING, ptr );
}


object
force_( object it ){
  if(  it.t != SUSPENSION  ) return  it;
  struct suspension *suspension = get_ptr( it );
  return  force_( suspension->f( suspension->env ) );
}

object
force_first( object it ){
  it = force( it );
  return  first( it );
}

object
first( list it ){
  if(  it.t == SUSPENSION  ) return  Suspension( it, force_first );
  if(  it.t != LIST  ) return  NIL_;
  struct list *list = get_ptr( it );
  return  list->a;
}

object
rest( list it ){
  if(  it.t != LIST  ) return  NIL_;
  struct list *list = get_ptr( it );
  return  list->b;
}
