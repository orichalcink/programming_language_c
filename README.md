# Programming Language
Programming language made in C by chalcinxx.

## How it works
Firstly, the input is retrieved from the user, if it is a file, the contents of the file are extracted instead. Then the user's input is converted to tokens through the lexer. After that the preprocessor handles imports and macros. That is everything that happens at the moment.

## Features
### Comments:
Comments are the same like in any C-style language:
```c
// This is a single line comment.
/*
 * This is a
 * multi line
 * comment.
 */
mut string name; // Your name
```
Tips: it does not matter what style you use, as long as you are consistent. Do not over-comment nor under-comment your code, as either makes it harder to understand it.
### Macros:
Macros can be created using the `define` keyword:
```c
define Degrees = float;
define Radians = float;

Degrees angle = 90.0;
Radians angle = angle * (3.14 / 180.0);
```
After processing the macros the file will look something like this:
```c
float angle = 90.0;
float angle = angle * (3.14 / 180.0);
```
The macros get processed after lexing, before parsing, so they cannot take the user input or any mathematical expressions. Also note that the macro will not resolve variables and put their value, but instead their identifier. Also note that only one token can be passed as an argument in a macro call, more on that later.
```c
define OnePlusTwo = 1 + 2;
let six = OnePlusTwo * 2;
```
would not get processed to this:
```c
let six = 3 * 2;
```
but instead to this, which might not be what the user has intended:
```c
let six = 1 + 2 * 2; // Oops, we got 5 instead of 6!
```
Macros can also include parameters. Macro call arguments must match macro definition parameters:
```c
define max(a, b) = (a > b ? a : b);
define min(a, b) = (a < b ? a : b);

max(10, 40); // 40
min(10, 40); // 10
```
`define` keyword read the macro up until the first semicolon, which means that this:
```c
define vars = let x = 20; let y = 30;
vars
```
will produce this, which might not be what the user has intended:
```c
let y = 30; // Since `define` ends at the first semicolon, this gets ignored and interpreted as a different statement
let x = 20 // Oops, order is incorrect and the semicolon is missing!
```
This is why `define_line` keyword exists, instead of stopping at the first semicolon, it goes until the end of the line, like a single line comment.
```c
define_line vars = let x = 20; let y = 30;
vars
```
expands to:
```c
let x = 20; let y = 30; // Do note that the last semicolon is included unlike in the `define` macro
```
Summary: macros get processed after lexing, but before parsing and interpretation, so mathematical expressions, function calls and user input are off limits. Macros can have parameters or not. Macro definitions have two types: `define` and `define_line`. Macro call arguments must be one token in size (10, identifier, "string", 'c') and their count must match the macro definition parameters.

Tips: only use non-parameter macros for type aliases and constants, like `using` and `constexpr` in C++. For parameter macros use parentheses around the macro body to prevent precedence issues (like the OnePlusTwo issue).
### Imports:
Other files can be imported using the `import` keyword:
>include.q
```c
define thing = mut string;
```
>script.q
```c
import "include.q"; // Imports MUST end with a semicolon
thing name;
```
Multiple files at once can be imported by separating them by a comma:
>include1.q
```c
define prefix = mut;
```
>include2.q
```c
define type = string;
```
>include3.q
```c
define identifier = name;
```
>script.q
```c
import "include1.q", "include2.q", "include3.q";
prefix type identifier;
```
Summary: imports can be defined using the `import` keyword, multiple files can be imported by separating them with commas. `import` statements like all statements should end in a semicolon.

