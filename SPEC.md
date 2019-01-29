Specification for the Jeru Language

## 1 Types
### 1.1 Types
Jeru implementations must support three types. An integer data type, a floating-point data type, and a string datatype.
The maximum size of integers and the accuracy of floats are both implementation defined.
The string datatype's encoding is implementation defined.

### 1.2 Literals
All three types described in 1.1 have literals.
When a literal is encountered, it should be pushed to the stack immediatly.
#### 1.2.1 Integers
An integer literal contains numbers 0-9, and may not have a minus sign. The first non-integer character terminates the reading of
a number. No seperating space is needed between the last digit and the next token.
Base modifiers, such as the `0b` prefix for binary, may or may not be included. Leading zero's may be allowed, but are not
gaurenteed to.

An example of a valid integer literal: `00123`
#### 1.2.2 Floats
A floating point literal contains exactly one decimal point (`.`) and optionally the numbers 0-9 on either or both sides of the
decimal. If no numbers are present on the left or right sides of the decimal, 0 is assumed (`0.0` = `0.` = `.0` = `.`).
Exponent notation, e.g. `12e+4`, are not be supported. Leading zero's may be allowed, but are not gaurenteed to. The first
non-integer character terminates the reading of a number. No seperating space is needed between the last character and the next
token.

An example of a valid float literal: `03.14159`
#### 1.2.3 Strings
A string literal is delimited by double quotes (`"`), and can contain the following escape sequences:

|     Name     |    Symbol    |
|--------------|:------------:|
| Newline      |     `\n`     |
| Tab          |     `\t`     |
| Double quote |     `\"`     |
| Backslash    |     `\\`     |

No seperating space is needed between the closing quote and the next token.
