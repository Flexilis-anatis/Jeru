# Specification for the Jeru Language

## 0 Common Definitions
This is a list of general information refrenced without citations throught the specification.
### 0.1 Stacks
Stacks are sequences of data that may be manipulated. Jeru implementations must have two:
#### 0.1.1 The Data Stack
The "data stack", or just "stack", is the area where every value[1.2] is stored. It must be theoretically infinite. When something is "pushed to the stack," it means it is appended to the end of the data stack, unless specified otherwise. When something is "popped from the stack," it means deleted from the end of the data stack, unless specified otherwise.

#### 0.1.2 The Code Block Stack
The "code block stack", or just "code stack", contains groupings of code. Everything stored on the code stack should be re-runnable, without any side effects. With a data stack of `[2]`, if I push the code block `1 +` to the stack, I should be able to run it twice to get the data stack `[4]`. Code blocks are discussed in greater detail in the section [1.2.4]

### 0.2 Whitespace
Whitespace is defined as any of the characters tab (escape `'\t'`), newline (escape `'\n'`), carriage return (escape `'\r'`), space (`' '`), or EOF characters. Whitespace is used to delimit tokens.

### 0.3 Tokens
A token is a literal[1.2] or a word[2].

## 1 Syntax
### 1.1 Types
Jeru implementations must support three types. An integer data type, a floating-point data type, and a string datatype.
The maximum size of integers and the accuracy of floats are both implementation defined.
The string datatype's encoding is implementation defined.

### 1.2 Literals
All three types described in [1.1] have literals.
Unless specified otherwise, literals are pushed to the data stack when they are encountered.

#### 1.2.1 Integers
An integer literal contains numbers 0-9, and may not have a minus sign. The first non-integer character terminates the reading of
a number. No seperating whitespace is needed between the last digit and the next token. Base modifiers, such as the `0b` prefix for binary, may or may not be included. Leading zero's may be allowed, but are not
gaurenteed to.

An example of a valid integer literal: `00123`
#### 1.2.2 Floats
A floating point literal contains exactly one decimal point (`.`) and optionally the numbers 0-9 on either or both sides of the decimal. If no numbers are present on the left or right sides of the decimal, 0 is assumed (`0.0` = `0.` = `.0` = `.`). Exponent notation, e.g. `12e+4`, is not be supported. Leading zero's may be allowed, but are not gaurenteed to. The first non-integer character terminates the reading of a number. No seperating whitespace is needed between the last character and the next token.

An example of a valid float literal: `03.14159`
#### 1.2.3 Strings
A string literal is delimited by double quotes (`"`), and can contain the following escape sequences:

|     Name     |    Escape    |
|--------------|:------------:|
| Newline      |     `\n`     |
| Tab          |     `\t`     |
| Double quote |     `\"`     |
| Backslash    |     `\\`     |

No seperating whitespace is needed between the closing quote and the next token.

#### 1.3 Code Blocks
Code blocks can only be pushed to the code stack. They are series of tokens delimited by the words(!) `[` and `]`. Note that they are words, not special symbols, so all the parsing rules in [2.1]
apply. When they are encountered, they should be pushed directly to the code stack, NOT the data stack.

Code blocks can be nested.

### 1.4 Number Promotion
When a integer[1.2.1] and a float[1.2.2] are operated on, the integer is generally cast to a float before any calculations are made.

This only applys to builtin math functions like `+` and `*`, and there are exceptions to this rule.

### 1.5 Truthy
A value is truthy if any of the following are true:
- The type is `float` or `int` and the value is not `0`
- The type is `str` and the value is not the empty string

### 1.6 Comments
Comments are anything between two hash marks (`#`). No whitespace is needed to seperate the contents of the comment from the hash marks. Whitespace is needed to seperate words from the first hash mark, but not the following word from the last hash mark (e.g. `#comment#some_word` works, but `some_word#comment#` does not).

## 2 Words
### 2.1 Definition
A word is a name associated with a block of code or an action. A word is defined as anything that's not a literal[1] or whitespace. If a hash mark is the second or later character of a word, it is not treated as a comment. If a word is found, it is first checked against builtin words[2.2]. If it does not match any of the builtin words[2.2], it is assumed to be a user-defined word[2.3].

### 2.2 Builtin words
The format in this section is:

`name (input types)(input blocks) -> (output types)(output blocks)`: description

The section `(input types)` is a comma-seperated list of the types `int` (meaning integer[1.2.1), `float` (meaning floating- point[1.2.2]), or `str` (meaning string[1.2.3]). The section `(input blocks)` defines how many code blocks should be on top of the stack. If ommitted, it is zero. When comma seperated, multiple types are implied. The item at the end of the type list should be the type at the end of the stack; that is, `(int, string)` would match the stack `[3.4, 5, "hoola-hoop"]`.

The section `(output types)` follows the same rules `(input types)`, but it is what is on top of
the stack after the word has run.

The section `(output blocks)` follows the same rules as `(input blocks)`, but it is what is on top of the code stack after the word has run.

If any of them contain the symbol `*`, it means it can be anything, including nothing. If `(output blocks)` or `(input blocks)` contain the symbol `+`, it means it is at least the amount specified.

When they are seperated by the `|` symbol instead of commas, it means either of them will suffice.

### 2.2.1 Number Operations
Unless otherwise specified, these words perform standard number promotion[1.4].

`+ (int|float,int|float) -> (int|float)`: adds two numbers.

`- (int|float,int|float) -> (int|float)`: subtracts two numbers

`* (int|float,int|float) -> (int|float)`: multiplies two numbers

`/ (int|float,int|float) -> (float)`: divides two numbers. All operands are cast to floats.

`> (int|float,int|float) -> (int)`: pushes `1` to the stack if the first number is larger than the second, `0` otherwise

`< (int|float,int|float) -> (int)`: pushes `1` to the stack if the first number is smaller than the second, `0` otherwise

### 2.2.2 Data Stack Operations
`copy (int|float|str) -> (int|float|str,int|float|str)`: copies the item on top of the stack

`pop (int|float|str) -> ()`: deletes the item on top of the stack

`print (int|float|str) -> (int|float|str)`: prints the top of the stack and leaves it unchanged.

### 2.2.3 Code Stack Operations
`exec ()(1) -> (*)(0+)`: executes and deletes the top of the code stack

`run ()(1) -> (*)(1+)`: executes and maintains the top of the code stack

### 2.2.4 Control Flow
`if (int|float|str)(1) -> (*)(0+)`: checks if a condition is truthy[1.4]. If it is, run's code block.

`ifelse (int|float|str)(2) -> (*)(0+)`: checks if a condition is truthy[1.4]. If it is, run's the first code block. Otherwise run's the second.

`while ()(1) -> (*)(0+)`: runs the block until until the top of the stack is not truthy[1.4]. In C terms it's a do-while loop, as it always runs the code at least once.

### 2.2.5 String Operations
`+ (string,string) -> (string)`: concatenate two strings

`* (string|int,string|int) -> (string)`: multiply a string by a number (E.G., `"*" 8 *` and `"*******"` are equivilant).

### 2.3 Syntactical Definition of Custom Words
Words can be defined with the following syntax:

`'[' (literal|word)* ']' 'word' word_name`

Note that whitespace is needed between the `'['` character and the code it contains, the `']'` character, the code it contains, and the word `'word'`, and the word `'word'` and the word name, which is parsed according to the rules in section [2.1]

### 2.4 Invocation of Custom Words
Words are invoked by simply stating their name. For instance, if you wanted a word to increment the top of the stack by 1, you could define the word as this:
```
[
    1 +
] word inc
```
or like this:
```
[ 1 + ] word inc
```
Or any combination of whitespace as long as tokens are properly seperated.

And call quite simply like this:
```
5 inc print # prints 6 #
```