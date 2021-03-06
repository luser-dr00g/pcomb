# pcomb
parser combinators in PostScript and C

Some experiments in implementing parser combinators in C and Postscript.

The PostScript versions all build upon the function syntax extension
described at
https://codereview.stackexchange.com/questions/193520/5912/an-enhanced-syntax-for-defining-functions-in-postscript
and implemented in struct2.ps. An older version, struct.ps, is in the old directory.

pc8.ps finally implements the idea of the parsers actually returning
useful values.  pc8token.ps and pc8re.ps implement a (partial)
PostScript `token` implementation and regular expression parser
(producing a parser for the pattern).

pc9.ps implements much the same behavior, but building the parsers
according to the Monad concept following Hutton and Meijer, "Monadic
Parser Combinators".  This has made the code shorter and simpler, and
the exercises pc9token.ps and pc9re2.ps demonstrate the usage. The
tokenizer is about 100 characters shorter than the pc8 versions for
the same functionality, but the regex code is about 500 characters
shorter.

The first C version was posted to
https://codereview.stackexchange.com/questions/166009/5912/parser-combinators-in-oo-c .

The next C version, which should be pc6.{c,h}, will attempt to implement
the same behavior as pc9.ps in C following the example of the small exercise lazy.c.

...Sigh, pc6 didn't work out. pc7 kinda worked, but I rewrote it as pc8 and that one
worked but it did not scale well. Memory usage and execution speed very quickly got
ridiculous when parsing anything longer than 5 or 6 lines.

So, pc9 finally implements lazy evaluation in the parsers, particularly the `plus`
combinator which handles alternates. It tries the first one and returns a Suspension
for the remainder.
