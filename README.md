# Parser Combinators in C, version 11.

## Objects

The first thing to remember when working with this code is the fact that many of
the types are hidden pointers. `object`, `list`, `parser`, etc. are all pointers
to a `union object` which has a specific `struct` inside depending on the specific
subtype indicated by the `tag t`. The intent is to make C look simpler, like other
dynamic languages with dynamically-typed variables. Using the power of the `typedef`
to suppress extraneous detail and "abstract" away from too much low level information.
So, treat all of these types as "references", using "*reference semantics*".

There is encapsulation, but no information hiding beyond the effort to supply
convenient functions for common tasks. For the most part, the objects look and
behave like local variables. You create them with a constructor, which you'll want
to do because otherwise the boilerplate looks so heinous. And then just discard them.
No deallocation or destructor to call. It's only in places where something tricky
needs to happen that any of these objects will have the arrow `->` operator applied
to them to peek at the values in the union object. So, treat them like handles. 
It's a thing, with more stuff behind it. One important place where the "pointerness"
of objects is significant is when creating loops in a parser graph using a `forward()`
parser, discussed more below. Also in the treatment of suspensions which lazily 
manifest new content for an object by overwriting the `union object` data.

The second unique landmark to mention is the use of the "McIllroy convention" for
include guards. In this style, the guards are placed around the `#include` directive.
With dutiful adherence to this convention, the system achieves the same effect as
`#pragma once` with the additional virtue that the preprocessor need not even look at
the file a second time. The application layer code simply `#include`s the outermost
header file for the suite of functions desired and the headers will include each
other as necessary. Consequently, the outermost `#include` directive for a program
doesn't need a guard. Avoid double inclusion at the top layer by simply not doing that,
please.

The object module -- `pc11object.h` and `pc11object.c` -- defines and exports the
smorgasbord of types of object and functions for constructing and
manipulating objects.

## Boolean objects

There are two global objects for the values of the stereotypical Lisp true/false type.
The `boolean` subtype is a "virtual" or *synthetic* type with 2 possible elements,
one of which is the symbol T, and the other is an invalid object which is also 
used as a sentinel to terminate lists.

    extern object T_   /* = (union object[]){ {.t=1}, {.Symbol={SYMBOL, T, "T"}} } + 1 */,
                  NIL_ /* = (union object[]){ {.t=INVALID} } */;

`valid()` is a helpful function for using the boolean type in C conditions or
otherwise checking that the object is in fact a pointer to something interesting.

    /* Determine if object is non-NULL and non-NIL.
       Will also convert a boolean T_ or NIL_ to an integer 1 or 0.
     */
    static int
    valid( object it ){
      return  it
	  &&  it->t <  END_TAGS
	  &&  it->t != INVALID;
    }


## Constructors

Each of these constructors allocate and initialize a `union object` and yield
the pointer to it.

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
      reduce( cons, PP_NARG(__VA_ARGS__), (object[]){ __VA_ARGS__ } )


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

There are three subtypes of `operator`: a "plain" operator, a predicate, or a binary
operator. The three function types all have compatible prototypes, so for expediency
all three use the same `.Operator` struct in the union object, and we rely upon 
"duck typing" to some degree to use the appropriate ones in appropriate places.

## Lists

A `list` can mean a couple of different things. In all cases it designates an object
whose tag is `LIST` (unless it's NIL: valid() will tell you) and contains pointers
to `first` and `rest` objects. A *proper list* is a data format built out of list
objects where the `first` pointer points to any type and is an element of the list,
and `rest` pointer leads to the next `LIST` type node or NIL.

The same `list` data structure can also be used to build a binary tree by relaxing
the requirement for the `rest` pointer to always be another `list` node. `first` and
`rest` are exactly the `car` and `cdr` functions from Lisp, but with more readable
names.


## Symbols

Symbol objects are constructed by passing an enum name to a macro
that expands both the value yielded by evaluating the enum name
as well as the enum name's string representation.

Symbol codes are allocated statically by defining each in an enum.
The object module defines its internal symbol names and then the
name `END_OBJECT_SYMBOLS` which the next module *layer* will use
to start off its own range of enum codes.

    enum object_symbol_codes {
    T,
    //...,
    END_OBJECT_SYMBOLS
    };
    
These are considered "compile time" symbols, although constructing a `symbol`
object happens dynamically, at run-time. 

Symbol codes are also allocated dynamically, when calling the `symbol_from_string`
function. This function searches through the allocation list for a Symbol object
whose printname matches the string. If there is no such object found, then a new
code is assigned in the space of negative integers below -1 (which == EOF).

There is obvious room for improvement in the efficiency of the dynamic symbols.


## Function objects

`parsers`, `operators`, and `suspensions` contain a saved environment and
a function pointer, as well as a printable string to hold the function's name.

Function objects are executed by calling the function pointer and passing it
the saved environment as its first argument and any other input as its
second argument. A supension function receives only its environment: there 
is no other input.

In the special cases of `bind` and `into` combinators, the right hand function will
be called with a modified enviroment, supplemented to contain the definition
(key.value) of the result of `into`'s left hand parser.


## Allocation of objects

All objects are allocated as an array of 2 union objects. The left one is
used as an allocation record and uses the `.Header` member of the union object.
The right one is the payload for the object. The `object` passed around and
used everywhere is a pointer to this *right hand side* object. By making the
allocation record the same size as any object (they're defined in the same union),
I can do this simple pointer shenanigan without worrying about alignment issues.

The allocation records are linked together in a list with the head of the list 
pointed to by the static file scoped `allocation_list`. This variable and all of 
the `next` pointers in the headers point to the left hand side.

Presently, only the `new_()` and `symbol_from_string()` functions use this list.
With a more efficient mechanism for dynamic names, this number will go down. With
the introduction of a garbage collector, this number will go up.

For the needs of the potential garbage collector, the T symbol needs a dummy
allocation record so the potential `mark` function can twiddle its flag.


## Parsers

The parser module -- `pc11parser.h` and `pc11parser.c` defines the parser behavior
and the combinators.

The Parser Combinators have been designed according to the plan from Graham Hutton's
paper, "Higher Order Functions For Parsing" and influenced by the seminal work,
"Recursive Programming Techniques" by Burge. The most basic parser, the "leaf" node 
in any parser graph is `satisfy(predicate)`.

    parser  satisfy( predicate pred );
    
When activated with a call to `parse()`, the `satisfy` parser forces one element
off of the front of the (possibly lazy) input stream, then tests the element with
the predicate closure, if true the parser succeeds, otherwise the parser fails.

A parser that succeeds will return a little `cons` tree.

    ( OK . ( <value> . <remainder of input> ) )

For the `satisfy` parser, the value is the input element itself. If the input stream
was constructed with `chars_from_str` or `chars_from_file`, the element will be an
`integer` object containing the character code in its payload.

A parser that fails will return a slightly different little `cons` tree.

    ( FAIL . ( <error message> . <remainder of input> ) )

The default error message from a failing `satisfy` parser is to print the name of
the predicate that failed and a string saying "predicate failed".

The value portion of the result returned by any parser can be modified by
`bind`ing the parser to an operator. The operator will be used to transform
the value portion returned by a successful parse.

Parsers can be combined into a choice with the `either` or `ANY`
combinators.  Parsers can be combined into a sequence with the `then`
or `SEQ` combinators.  Two variants of `then` will delete the result
from the left hand parser (`xthen`) or delete the result from the
right hand parser (`thenx`). The special sequencing combinator `into`
will make the result from the left hand parser available to the right
hand parser (and importantly to its `bind`ed operator, if it has one)
by defining the value in the right hand parser's environment with a
specified key.

The `ANY` and `SEQ` combinators take advantage of the fact that a
combinator whose signature is `parser function( parser, parser )` is
naturally compatible with `object function( object, object )` and so
it can be treated as a binary operator function for use with
`fold_list()` or `fold_array()`.  The implementation for these mirrors
the implementation of the `LIST()` macro which folds over an array
using `cons` as the binary operator (since `list` is also compatible
with `object`).

A few combinators provide optional repetitions. `maybe` succeeds if its child parser 
matches 0 or 1 times. `many` succeeds if its child parser matches 0 or more times.
`some` succeeds if its child parser matches 1 or more times.

Two simple compilers can convert a `regex` to a parser or a block of `ebnf` definitions
into an association list of `(symbol.parser)` pairs.


## Parser symbols

The parser module defines a number of internal symbol names and provides the name 
`END_PARSER_SYMBOLS` for the next layer to create more unique symbol codes.


## IO module

As an illustration of the power of the combinator approach, the task of 
creating a string-based DSL for formatted input and output can be modelled
as a parsing exercise. The IO module -- `pc11io.h` and `pc11io.c` -- contains
an implementation of a subset of `printf()` and `scanf()`
named `pprintf()` and `pscanf()`.

And for now, this is the outermost layer of the library. The IO module provides the
name `END_IO_SYMBOLS` for the next layer to create more unique symbol codes.


## Test module

The test module -- `pc11test.h` and `pc11test.c` illustrates simple usage of the
list objects and parsers, building parsers from `regex`es,
and using the `ebnf` compiler.


## Debugging

A decorator function `parser z = probe( p, mode );` can provide feedback
on what parser `p` is doing when `z` is run.
`mode == 1` will print out the (intermediate) result of a successful parse.
`mode == 2` will print out the (intermediate) failure result of a failed parse.
`mode == 3` will print both.
`mode == 0` just calls p and returns its result and doesn't print anything.

Similarly, if an operator attached with `bind` is having trouble massaging the
desired result, try just calling `print` on its input to see what kind of object
or list structure it was given.

`print()` will print out a representation for any object. It will print list
structures with the Lisp "dot notation" showing all the list nodes and their
`first` and `rest` parts separated by a `'.'`.

`print_list()` will print out a representation for any object as with `print()`
(it calls `print()` directly for anything not a list). It will print a list
structure with the Lisp "list notation" where a chain of list nodes linked
together along their `rest` pointers will be printed simply with one opening
parenthesis and one closing parenthesis.

Some additional behaviors of the print*() functions are controlled by file
scoped static flags in `pc11object.c` such as
whether to print integers as a decimal number 
or interpreted as an ascii character and presented in single quotes,
or whether to print a symbol's code number as well as the symbol's name,
or whether to dump the innards of a function object and print its environment
in dot notation.


## Building Recursive Parser Graphs

The parser constructed with `forward()` can be composed with `then` and `either`
and then have its value filled in by the resulting "higher level" parser. This
creates a loop in the parser graph. Take care not build a graph with left recursion
or the recursive descent parsing algorithm will become locked in an infinite
unproductive loop.

For examples of this, see the implementations of the `many` combinator and
the `regex_grammar()` and `ebnf_grammar()` functions in `pc11parser.c`.

To avoid infinite recursion, the `print()` function will refuse to print the
innards of a parser created with `forward()` -- even after its value has been
overwritten -- and will display only its printname.


## Building Concrete Syntax Trees (CST)

Since the behavior of `then` is to merge the values from the left and right parsers
into a list, the default behavior of any graph of parsers is to yield a flat list
of all matched input elements (excluding those deleted by `xthen` or `thenx`).
In order to introduce any substructure to this list, it is necessary to use
`bind` to transform the values yielded by certain parsers.

A simple operator function like

    static list
    encapsulate( object env, object input ){
        return  one( input );
    }

will enclose the value by wrapping an additional list structure around it
when `bind`ed to a parser. If this parser is combined as a child of `then`,
the default merging of results will leave this substructure unmolested.
So, by peppering a parser graph with calls to 

    bind( <parser>, Operator( NIL_, encapsulate ) )
    
one can wrap any desired grammatical parts of the input in its own sub-list,
building a tree with N-way branching.

Or this task could be factored into a new decorator.

    static parser
    enclose( parser p ){
        return  bind( p, Operator( NIL_, encapsulate ) );
    }

And then pepper the parser graph with calls to

    enclose( <parser> )

which is shorter if you're gonna do it a lot.


## Separating The Lexer and Parser passes

You can fashion a parser into a syntax directed compiler for
the *tokens* of a language generating a list of symbols.
By following the model of `chars_from_str()` this list could
be supplied lazily by a function that yields one token at a time.

A syntax directed compiler is made by constructing a parser
graph by composing parsers with combinators, and have this
graph peppered with calls to `bind()`. The attached operators
must accept an integer or list of integers and yield the desired
artifact. For the present case of a tokenizer, these artifacts
will be `symbol` objects. After a symbol is constructed by a call
to its constructor `Symbol()` or `Symbol_()`, it has an extra
`object data` pointer inside of the `.Symbol` struct in the union object.
You can stash any extra info you like in the symbol's `->data`.
It will be invisible to the syntax analysis layer.

Then the syntax analysis layer can be constructed to recognize
`symbols` as its input elements. You will need to write predicates
which call `eq_symbol` to check symbol codes, and call `satisfy()`
to produce parsers from the predicates, and populate an enum
with all necessary symbol codes. The first enum name ought to be
initialized to the `END_*_SYMBOLS` name from the outermost layer
of the library that is used.

