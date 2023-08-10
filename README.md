# Winforth

A Forth implementation in C++, built on Windows 98.

## What is Forth?

Forth is a family of languages with many implementations, known for being minimal and to work mainly by manipulating certain data structures such as a stack and a dictionary.

The dictionary is comprised of user-defined words. Defining a word is to associate another set of words and commands to said word so that, during execution, the defined word is substituted by the given set of words that determine its meaning.

## Reasoning

This is a simple hobby project for implementing a Forth interpreter under Windows 98, using Visual C++ 6.0. The mentioned IDE is in no way limited, but I wanted to explore how much I could do with Visual C++ 6.0, given the well-known fact that this implementation is not 100% compliant with the C++ spec at the time -- in fact, I heard that the STL is not even thread-safe.

I am also providing a Makefile for building this project on Linux.

## Documentation

This Forth implementation has the following characteristics, that might be less orthodox:

- It is case-sensitive;
- There is only integer arithmetic;
- There is no text or strings;
- There are no memory operators.

Part of these characteristics are derived from implementation constraints (such as time or laziness), and may change anytime.

### Primitive words

This Forth implementation is case-sensitive, and has the following built-in words:

- `true`: Evaluates to -1 (all bits set).
- `false`: Evaluates to 0 (no bits set).
- `.`: Pops the topmost value and prints to console.
- `dup`: Push a copy of the topmost stack value.
- `swap`: Switch places between te two topmost stack values.
- `rot`: Rotate the three topmost stack values. A stack containing numbers 1, 2, 3 (rightmost is stack top) becomse 2, 3, 1 -- in other words, the third number on top is moved to the top.
- `over`: Duplicates the second number on top and pushes it. A stack containng numbers 1, 2 (rightmost is stack top) becomse 1, 2, 1.
- `drop`: Pops the topmost value.
- `and`: Consumes the two topmost values performing an "and" operation and pushes the result. This is also a bitwise operation.
- `or`: Consumes the two topmost values performing an "or" operation and pushes the result. This is also a bitwise operation.
- `xor`: Consumes the two topmost values performing an "exclusive or" operation and pushes the result. This is also a bitwise operation.
- `invert`: Consumes the topmost value performing a "not" operation and pushes the result. This is also a bitwise operation.
- `+`, `-`, `*`, `/`: Arithmetic operators. For division, the second operator must not be zero.
- `<`, `<=`, `<>`, `>`, `>=`, `=`: Words for comparing integer numbers. These words consume the two topmost values and push the result.
- `.s`: Prints the stack to console.
- `emit`: Prints a character to console. Topmost value is considered the ASCII decimal value of the character.
- `!`: Stores a value on a variable. See the "Variables" section below for usage.
- `@`: Retrieves the value from a variable. See the "Variables" section below for usage.
- `bye`: Exits the interpreter.

Comparison and logic/bitwise words conform to pushing values like `true` or `false`, or `-1` and `0` respectively. Because of the binary representation of these values, the logic words also work seamlessly as bitwise operations.

### Other syntax

#### Variables

In Winforth, variables are global values identified by a positive integer address. Each variable is declared through a textual alias, like the following example:

```fth
variable foo
variable bar
```

After declaring a variable, you can reference it by name; referencing it will push the variable's address, which is just a positive integer value, so please be mindful of that.

The default value of any new variable is 0; the value of a variable can be queried by using the word `@` with the following syntax:

```fth
<varaddr> @
```

Similarly, a new value can be stored on a variable by using the word `!` with the following syntax:

```fth
<value> <varaddr> !
```

The following example creates a variable `sum`, stores the value of `2 * 3` in it, queries the value by pushing it onto the stack, and finally pops and prints the stored value.

```fth
variable sum

2 3 * sum !
sum @ .
```

#### Comments

```fth
( This is a comment. )
```

Comments are delimited by parentheses (which must be surrounded by white space). They will be ignored by both the parser and the evaluator. A comment that starts in a line and ends in another one is considered a syntax error on the REPL.

#### Declaring words

```fth
: <word-name> <definition...> ;
```

New words can be added to Winforth's dictionary by using a word declaration syntax.

A word definition always starts with `:`, followed by a string that names the word. These two tokens are then followed by a series of any number of words, and will be the definition of the word being declared. Finally, a `;` token delimits the end of word definition.

Contrary to comments, word definitions can be multiline on the REPL.

It is a common pattern to describe the state of the meaningful portion of the stack on a comment in front of the word name:

```fth
: square ( x -- x2 )
  dup * ;

: sum-of-squares ( a b -- r )
  square swap square + ;
```

#### Conditionals

```fth
<predicate> if <consequent...> then
<predicate> if <consequent...> else <alternative...> then
```

There are two kinds of conditionals. The first one defines what to do when a certain condition is met; the second one does that, but also determines what to do otherwise.

The syntax of a conditional can be unorthodox in many levels. It is important to remember that a conditional such as this one in C++...

```cpp
if(value >= 0)
    return 1;
else
    return -1;
```

...can be written in Forth this way:

```fth
( value is topmost on stack )
0 >= if 1 else -1 then
```

## License

This project is distributed under the MIT License. See LICENSE file for more information.
