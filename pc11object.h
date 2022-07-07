#define PC11OBJECT_H
#include <stdlib.h>
#include <stdio.h>
#if ! PPNARG_H
  #include "ppnarg.h"
#endif


/* Variant subtypes of object,
   and signatures for function object functions */

#define IS_THE_TARGET_OF_THE_HIDDEN_POINTER_  *
typedef union    object  IS_THE_TARGET_OF_THE_HIDDEN_POINTER_  object;

typedef object   integer;
typedef object   list;
typedef object   symbol;
typedef object   string;
typedef object   boolean;

typedef object   suspension;
typedef object   parser;
typedef object   operator;
typedef operator predicate;
typedef operator binoperator;

typedef object   fSuspension( object env );
typedef object   fParser( object env, list input );
typedef object   fOperator( object env, object input );
typedef boolean  fPredicate( object env, object input );
typedef object   fBinOperator( object left, object right );


typedef enum {
  INVALID,
  INT,
  LIST,
  SYMBOL,
  STRING,
  VOID,
  SUSPENSION,
  PARSER,
  OPERATOR,
  END_TAGS
} tag;

enum object_symbol_codes {
  T,
  END_OBJECT_SYMBOLS
};


struct integer {
  tag t;
  int i;
};

struct list {
  tag t;
  object first, rest;
};

struct symbol {
  tag t;
  int code;
  const char *printname;
  object data;
};

struct string {
  tag t;
  char *str;
  int disposable;
};

struct void_ {
  tag t;
  void *pointer;
};

struct suspension {
  tag t;
  object env;
  fSuspension *f;
  const char *printname;
};

struct parser {
  tag t;
  object env;
  fParser *f;
  const char *printname;
};

struct operator {
  tag t;
  object env;
  fOperator *f;
  const char *printname;
};

struct header {
  int mark;
  object next;
  int forward;
};

union object {
  tag t;
  struct integer Int;
  struct list List;
  struct symbol Symbol;
  struct string String;
  struct void_ Void;
  struct suspension Suspension;
  struct parser Parser;
  struct operator Operator;
  struct header Header;
};



/* Global true/false objects. */

extern object NIL_; /* .t == INVALID */
extern symbol T_;



/* Determine if object is non-NULL and non-NIL.
   Will also convert a boolean T_ or NIL_ to an integer 1 or 0.
 */

static int
valid( object it ){
  return  it
      &&  it->t <  END_TAGS
      &&  it->t != INVALID;
}


/* Constructors */


integer    Int( int i );

boolean    Boolean( int b );

string     String( char *str, int disposable );

object     Void( void *pointer );


/* List of one element */

list       one( object it );


/* Join two elements togther. If rest is a list or NIL_, result is a list. */

list       cons( object first, object rest );


/* Join N elements together in a list */

#define LIST(...) \
  fold_array( cons, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )


/* Macros capture printnames automatically for these constructors */

#define    Symbol( n ) \
           Symbol_( n, #n, NIL_ )
symbol     Symbol_( int code, const char *printname, object data );

#define    Suspension( env, f ) \
           Suspension_( env, f, __func__ )
suspension Suspension_( object env, fSuspension *f, const char *printname );

#define    Parser( env, f ) \
           Parser_( env, f, __func__ )
parser     Parser_( object env, fParser *f, const char *printname );

#define    Operator( env, f ) \
           Operator_( env, f, #f )
operator   Operator_( object env, fOperator *f, const char *printname );



/* Predicate combinators */

predicate  not( predicate p );

predicate  and( predicate p, predicate q );

predicate  or( predicate p, predicate q );



/* Printing */


/* Print list with dot notation or any object */

void    print( object a );


/* Print list with list notation or any object */

void    print_list( object a );



/* Functions over lists */


/* First element of list.
   Same as Lisp (car) */

object  first( list it );


/* Remainder of list after dropping the first element.
   Same as Lisp (cdr) */

list    rest( list it );


/* Apparent length of list.
   Does not force a suspension, but counts it as 0. */

int     length( list ls );


/* Force n elements from the front of (lazy?) list.
   Returns a copy. */

list    take( int n,
	      list it );


/* Drop n elements from the front of (lazy?) list */

list    drop( int n,
	      list it );


/* Index a (lazy?) list */

object  nth( int n,
	     list it );


/* Apply operator to (lazy?) object */

object  apply( operator op,
	       object it );


/* Produce lazy lists */

list    infinite( object mother );

list    chars_from_str( char *str );

list    chars_from_file( FILE *file );


/* Lazy list adapters.
   Extended to full width --
     ie. up to a 6-byte utf8 encoding representing a 32bit ucs4 code. */

list    ucs4_from_utf8( list o );

list    utf8_from_ucs4( list o );




/* Maps and folds */


/* Map elements of list it through operator function.
   Build a new list, each element of which is the result of
     apply( op, <corresponding element of list it> )
 */

list    map( operator op,
	     list it );


/* Fold right to left over list with f */

object  fold_list( fBinOperator *f,
		   list it );


/* Fold right to left over array of objects with f */

object  fold_array( fBinOperator *f,
		    int n,
		    object po[] );




/* Comparisons and Association Lists (Environments) */


/* Compare for equality. For symbols, just compare codes. */

boolean eq( object a,
	    object b );


/* Call eq, but avoid the need to allocate a Symbol object */

boolean eq_symbol( int code,
		   object b );


/* Return copy of head with a pointer to tail in place of
   head's terminating NIL. */

list    concat( list head,
		list tail );


/* Prepend n (key . value) pairs to tail */

list    env( list tail,
	     int n, ... );


/* Return value associated with key in env */

object  assoc( object key,
	       list env );


/* Call assoc, but avoid the need to allocate a Symbol object */

object  assoc_symbol( int code,
		      list env );




/* Conversions */


/* Return length of strings + number of integers in list.
   Ie. the size of string required to use fill_string().
 */

int     string_length( list it );


/* Copy integers and strings into *str. modifies caller supplied pointer */

void    fill_string( char **str,
		     list it );

/* Convert integers and strings from list into a string */

string  to_string( list ls );


/* Dynamically create a symbol object corresponding to printname s.
   Scans the list of allocations linearly to find a matching printname.
   Failing that, it allocates a new symbol code from the space [-2,-inf). */

symbol  symbol_from_string( string s );




/* That one lone function without a category to group it in. */


/* Report (an analogue of) memory usage.
   By current measure, an allocation is 64 bytes,
   ie. 2x 32 byte union objects.
   Largest object is 3 pointers + integer tag.
     ie. 3x 8byte pointers + 4byte integer + 4byte padding.
 */
   

int count_allocations( void );


