# The J Incunabulum Redux

The notorious "J Incunabulum" (a.k.a ["The One Page Thing"](https://www.jsoftware.com/papers/AIOJ/AIOJ.htm)) has been commented and explained in some places, for example, [here](https://github.com/tavmem/incun) and [here](https://github.com/tangentstorm/j-incunabulum/blob/master/README.md). Recently someone even asked [ChatGPT to explain it](https://medium.com/@solarbreeze69/chatgpt-explains-arthur-whitneys-j-incunabulum-5be2ea69a298).

This dense piece of C code was written by Arthur Whitney in 1989 as a proof of concept for what would later become [J](https://aplwiki.com/wiki/J), the new language then being designed by Ken Iverson. It is a bare-bones interpreter with minimum functionality. Its only literal data type is a positive one-digit integer and it provides no error checking whatsoever. However, within these strict restrictions it is able to show the basic features of an array programming language.

The interpreter only works if you give it a well formed expression. Mistype a single character and it will probably crash! And, by the way, it never reclaims unused malloc'ed cells. However, this is part of the game. If you get into the spirit of the PoC and explore its intended functionality you'll become amazed at how much it accomplishes with so little!

Unfortunately, the code was written a long time ago and it doesn't compile with a modern C compiler anymore. What follows is my attempt to bring it up to date and exercise it a bit.

## Updating the source to compile with C99

I left the original source code as `j.c` for easy reference and worked on a new copy, `nj.c`. I applied succesive batches of changes, each registered as a separate `git commit` until it compiled cleanly and I was satisfied. I started trying to compile it on macOS with

    % clang -std=c99 nj.c -o nj

This produced a long list of errors and warnings, so I introduced the following patches. I tried to make the changes as small as possible so as not to disturb the original style although I think I have failed this goal in the change related to `gets()` (see later).

### Add ANSI C function parameters

In the ancient, pre-ANSI, K&R C, function parameter types were declared after the parameter list and before the function body. With the advent of ANSI C, the types were moved to the parameter list itself. So, for example, the macro to define dyadic functions was changed from `#define V2(f) A f(a,w)A a,w;` to `#define V2(f) A f(A a,A w)`. Similar changes were made to several other functions.

Related changes

* Macro `V` was created to denote `void` and used as the return type of some functions
* Parameter types were added to the function pointer declaration in the function tables `vd[]` and `vm[]`
* `I` was added where before an implied `int` was expected in function return type or parameter

### Use system headers and fgets()

The usual system headers `<stdio.h>`, `<stdlib.h>` and `<string.h>` were included in order to supply prototypes for `printf()`, `malloc()` and `strlen()`. Function `gets()` is now considered deprecated and unsafe because it cannot check if an input line would overflow the buffer. It was replaced with `fgets()` with the minimum changes necessary to skip empty lines and not crash. Sorry, Arthur, this broke your tight one-liner, but it's all in the name of progress 😊.

### Resolve pointer/integer conflicts

The type `I` is defined as `long` because it was meant to hold either an integer or a pointer. This is a common trick when doing low level programming but it requires the use of some casts to keep the compiler from emitting warnings. Besides, the type for the array that stores the variables, `st[]`, was changed from `I` to `A` since it only stores pointers to arrays, never integers.

### Use sizeof(I) instead of 4 in malloc()

The call to `malloc()` used to multiply the item count by 4 in order to get a byte count. This hardwired constant indicates the author was using a 32-bit computer (Roger Hui [tells us](https://www.jsoftware.com/papers/AIOJ/AIOJ.htm) it was an [AT&T 3B1](https://en.wikipedia.org/wiki/AT%26T_UNIX_PC)). Since macOS is nowadays 64-bit-only and since we don't want to be dependent on this issue, replaced 4 with `sizeof(I)`.

### Fix pedantic errors and extra warnings

Added `V` as the parameter list for `nl()` and `main()`. Commented out function `find()`, whose empty body was causing unnecessary warnings.

### Final change

As I thought a bit more about the definition of `I`, I realized it could still be improved. First, the type `long` is usually big enough to hold a pointer but not always. If I remember correctly, on 64-bit VMS, `sizeof(long)=4` while `sizeof(char *)=8`. So perhaps `I` should be `long long`.

Second, on some operating systems (usually 32-bit systems, I guess), some pointers returned by `malloc()` might have the most significant bit turned on, which means they would be interpreted as a negative integer when cast to `(I)`. This might cause problems in `qp()` and `qv()` where a pointer might be compared to a small positive integer. We want the pointer to always be greater than those integers.

In the end, I decided to use `<stdint.h>` and typedef `I` as `uintptr_t`, an unsigned integer long enough to hold a pointer.

### Result so far

    % clang -std=c99 -pedantic-errors -Wall -Wextra nj.c -o nj

Compiles cleanly on macOS Sonoma/M2 using

    Apple clang version 15.0.0 (clang-1500.3.9.4)
    Target: arm64-apple-darwin23.5.0

## Testing the interpreter

The interpreter displays neither a welcome message nor a visible prompt.
It just sits there waiting for you to throw something at it.

    % ./nj

Type `<CTRL-D>` to exit.

### Nouns

As mentioned above, the only scalar types are single digit integers (really, just `0` to `9`).
An integer is evaluated to itself, so when you type one at the terminal, it is just printed back.

```
5

5 
```

### Pronouns

Variables (also known as pronouns) are denoted by lowercase letters, `a` to `z`.
You assign a value to a variable using the `=` symbol.

```
a=7

7 
a

7 
```

### Verbs

In J, operators are called verbs and can be of two types depending on whether they receive one or two operands:

* Monadic (a.k.a. unary)
* Dyadic (a.k.a. binary)

So, the symbol `=` used for assignment above is thus a dyadic operator.

In this J prototype, operators are denoted by a single character.
There are five of them and the same symbol is used for a monadic and a dyadic operator.
If the operator appears at the start of an expression, it is monadic and operates on the expression following it to the right.
Otherwise, the operator is dyadic and operates on the noun or pronoun immediately to its left and the expression following it to the right.

The five operator symbols are: `+{~<#,`

### Evaluation

When a dyadic operator recursively evaluates the (sub-)expression to its right, this process propagates through all dyadic operators until the end of the expression is reached. At this point the rightmost operand is given to the last operator in the expression. It computes its result and passes it to the previous operator in the chain and this is repeated until all the expression is evaluated.

This has the effect of actually evaluating an expression from right to left without any operator precedence, as in APL.

In the tradition of APL, the left operand is named `α` (alpha) and the right one `ω` (omega). In the ASCII world of J, `α` becomes simply `a` and `ω` becomes `w`.

So dyadic operators are typically described as `a <op> w` and mondic ones as `<op> w`.

### Monadic operator `+` (Identity)

Simply returns the expression to its right.

```
+9

9 
+a

7 
```

### Dyadic operator `+` (Plus)

Returns the element-wise sum of the operands. Works only if both operands have the same shape. Otherwise it can produce incorrect results or crash.

```
3+4

7 

a
3 
3 4 5 

b
3 
0 1 2 

a+b
3 
3 5 7 
```

### Monadic operator `~` (Iota)

Returns an array or rank 1 (i.e. a vector) with integers from `0` to `n-1`, where `n` is the value of the expression to the right of `~`.

```
a=7

7 

~a
7 
0 1 2 3 4 5 6 

~+5
5 
0 1 2 3 4 
```

Note that when displaying an array the interpreter first prints its shape, i.e. the vector with its dimensions. In the above examples both arrays have rank 1 and this only one integer is displayed, which corresponds to the vector's length.

### Dyadic operator `~` (Find)
There's a skeleton for this function in the code but it is not implemented.

### Monadic operator `,`
Not implemented.

### Dyadic operator `,` (Catenate)

Concatenates the left and right operands producing a linearized vector with all the elements.

```
a=5,2
2 
5 2 

b=~6
6 
0 1 2 3 4 5 

a,b
8 
5 2 0 1 2 3 4 5 

7,a,b
9 
7 5 2 0 1 2 3 4 5 
```

### Monadic operator `#` (Shape)

Returns the shape of the argument, that is, a vector with the lengths of each dimention. A scalar is considered to have length 0.

```
#5
0 

#4,5
1 
2 
```

### Dyadic operator `#` (Reshape)

Reorganizes the elements of the right argument according to the shape of the left argument. Unfortunately arrays of rank 2 or 3 are still displayed in a linearized vector form. If the right arguments has less elements than the number of elements in the resulting array, the extra elements are repeatedely taken from right argument.

```
x=2,3,4
3 
2 3 4 

y=~6
6 
0 1 2 3 4 5 

x#y
2 3 4 
0 1 2 3 4 5 0 1 2 3 4 5 0 1 2 3 4 5 0 1 2 3 4 5 

#x#y
3 
2 3 4 

z=2,2
2 
2 2 

z#0
2 2 
0 0 0 0 
```

### Monadic operator `<` (Box)

The box operator transforms its operand, be it a scalar or an array, into a new scalar with a distict type. Since arrays can only contain scalars, the idea is to enclose other arrays in boxes so that they can be used as elements of another array. In this J prototype a boxed item is displayed with the symbol `<` before it. It is possible to enclose an item in many levels of boxes.

```
a=2,3,4
3 
2 3 4 

b=~6
6 
0 1 2 3 4 5 

<5  

< 
5 

x=<a

< 3 
2 3 4 

y=<b

< 6 
0 1 2 3 4 5 

x,y
2 
< 3 
2 3 4 
< 6 
0 1 2 3 4 5 

<x 

< 
< 3 
2 3 4 


<<x

< 
< 
< 3 
2 3 4 

```

### Dyadic operator `<`
Not implemented

### Monadic operator `{` (Size)

If the operand is a scalar (rank == 0) returns 1. Otherwise returns the length of the first dimension of the array. If the array is a vector, this makes sense, but if it has ran 2 or 3 these last dimensions are ignored. I'm not sure if this is useful or correct. I would think that all the dimensions should be accounted for in the array size.

```
{9

1 

a=~6
6 
0 1 2 3 4 5 

{a

6 
 
b=2,3
2 
2 3 

c=b#a
2 3 
0 1 2 3 4 5 

{c

2 
```

### Dyadic operator `{` (From)

Indexes or selects an item from the right argument. The index of the item is given by the left argument.

```
a=8,7,6,5,4,3,2,1
8 
8 7 6 5 4 3 2 1 

0{a

8 

3{a

5 
```

If the right argument is a rank 2 array (matrix), the returned item is a line (vector).

```
a=2,3
2 
2 3 
b=~6
6 
0 1 2 3 4 5 

c=a#b
2 3 
0 1 2 3 4 5 

0{c
3 
0 1 2 
1{c
3 
3 4 5 
```

If the right argument is a rank 3 array, the returned item is rank 2 array (matrix).

```
a=2,3,4
3 
2 3 4 
b=~9
9 
0 1 2 3 4 5 6 7 8 

c=a#b
2 3 4 
0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 

0{c
3 4 
0 1 2 3 4 5 6 7 8 0 1 2 

1{c
3 4 
3 4 5 6 7 8 0 1 2 3 4 5 
```

## Final thoughts

After making it work as initially intended and playing with it a little, one is tempted to fix some of its deficiencies and add more features. For example, we could make it parse multi-digit integers, check for errors in various situations and emit nice messages, add a character data type, more functions, etc. However, this is beyond the point of the PoC. If you want a nice interpreter, check the full [J](https://www.jsoftware.com/#/) interpreter!



