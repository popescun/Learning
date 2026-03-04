# Phases of translation

## Translation process
The text of a C++ program is kept in units called source files.
C++ source files undergo translation to become a translation unit, consisting of the following steps:
1. Maps each source file to a character sequence.
2. Converts each character sequence to a preprocessing token sequence, separated by whitespace.
3. Converts each preprocessing token to a token, forming a token sequence.
4. Converts each token sequence to a translation unit.

## Preprocessing tokens
A preprocessing token is the minimal lexical element of the language in translation phases 3 through 6.

The categories of preprocessing token are:

- `header names` (such as <iostream> or "myfile.h")
- placeholder tokens produced by preprocessing import and module directives (i.e. import XXX; and module XXX;)(since C++20)
- `identifiers`
- `preprocessing numbers` (see below)
- `character literals`, including user-defined character literals(since C++11)
- `string literals`, including user-defined string literals(since C++11)
- `operators and punctuators`, including alternative tokens
- individual non-whitespace characters that do not fit in any other category
The program is ill-formed if the character matching this category is
  - `apostrophe` (', U+0027),
  - `quotation mark` (", U+0022), or
  - a character not in the basic character set.

## Preprocessing numbers
The set of preprocessing tokens of preprocessing number is a superset of the union of the sets of tokens of integer literals and floating-point literals:
...
A preprocessing number does not have a type or a value; it acquires both after a successful conversion an integer/floating-point literal token.

## Whitespace

Whitespace consists of comments, whitespace characters, or both.

## Maximal munch

The `maximal munch` is the rule used in phase 3 when decomposing the source file into preprocessing tokens.

If the input has been parsed into preprocessing tokens up to a given character (otherwise, the next preprocessing token will not be parsed, which makes the parsing order unique), the next preprocessing token is generally taken to be the longest sequence of characters that could constitute a preprocessing token, even if that would cause subsequent analysis to fail. This is commonly known as `maximal munch`.
```cpp
int foo = 1;
int bar = 0xE+foo;   // Error: invalid preprocessing number 0xE+foo
int baz = 0xE + foo; // OK
```
In other words, the maximal munch rule is in favor of `multi-character operators and punctuators`:
```cpp
int foo = 1;
int bar = 2;
 
int num1 = foo+++++bar; // Error: treated as “foo++ ++ +baz”, not “foo++ + ++baz”
int num2 = -----foo;    // Error: treated as “-- -- -foo”, not “- -- --foo”
```
The maximal munch rule has the following exceptions:
...

### Alternative tokens
There are alternative spellings for several operators and other tokens that use non-ISO646 characters. In all respects of the language, each alternative token behaves exactly the same as its primary token, except for its spelling (the `stringification operator` can make the spelling visible). The two-letter alternative tokens are sometimes called `"digraphs"`. Despite being four-letters long, `%:%:` is also considered a digraph.



Primary	Alternative
&&	and
&=	and_eq
&	bitand
|	bitor
~	compl
!	not
!=	not_eq
||	or
|=	or_eq
^	xor
^=	xor_eq
{	<%
}	%>
[	<:
]	:>
#	%:
##	%:%:

## Tokens

A token is the minimal lexical element of the language in translation phase 7.

The categories of token are:
- identifiers
- keywords
- literals
- operators and punctuators (except preprocessing operators)

## Translation phases

### Phase 1: Mapping source characters
### Phase 2: Splicing lines
### Phase 3: Lexing
### Phase 4: Preprocessing
### Phase 5: Determining common string literal encodings
### Phase 6: Concatenating string literals
### Phase 7: Compiling
### Phase 8: Instantiating templates
### Phase 9: Linking