# Current bugs

int test12 x is identified as dq ??
why does test12 compile btw
it should crash and say it can't store x in c

- [x] storing larger type in smaller type succeeds?
    FIX: the onlyright flag of typeCompatible() in parsePrimary() was not set
- [x] for some reason ints are treated as DQ in .data section ?
    FIX: The types were improperly parsed to m_initDataSize in
    genDataSection() in generator.cpp

- [ ] So currently some parsing errors exist in the function system
      for example you can declare functions like `void func(5, ddsfds);` and
      it would compile just fine (obviously it would crash when you try to use
      the function though) but a mature compiler should filter these errors out

- [x] We can't use add unsigned and signed types etc... the problem is i don't
      know if that's a bad thing
      it is, i just checked every compiler. the only thing that might be
      valueable is warning the user about the use of signed / unsigned divisions
      because the cause a ton of not amazingly defined behaviour ? i think ?
      i don't understand its behaviour at least. also i think gcc just
      says f you and turns the int to its complement or something

- [x] variable assignment like int x = 4564564 isn't for bounds checked if it is global
- [x] variable assignment like x = 123564654 isn't checked for bounds
- [x] for some reason we don't have to put the last }
- [ ] Major problem, if variables go over 0xFFFF FFFF they overwrite our internal value int, we could fix this by using longs but we need to change our whole damn system to long's

# Todos
- [x] return statement parsing
- [ ] return statement enforcing
- [ ] struct declarations pad all types (also if two bytes come after each other etc)
- [ ] parse type conversions (remove faulty ones like conversions from structs etc)
- [x] Pushing structs to variable argument functions is incorrect
- [ ] gotta fix the hardcoded include directory for our preprocessor

# Implemented / roadmap
Started 16/4/2020
- [x] [16/4/2020] operators + - * /
- [x] global variables
- [x] while loops
- [x] if / else statements
- [x] for loops
- [x] function declaration
- [x] local variables
- [x] negative numbers via -
- [x] function calling
- [x] & address pointer
- [x] Created a dedicated error handler instead of the terrible macros I used
- [x] added -Wconversion and -Werror flags and used getopt_long
- [x] conversion between int and unsigned now work automatically like in gcc
- [x] Octal, hexadecimal and binary numbers now work
- [x] pointers
- [x] scaling pointer values and pointer arithmetic
- [x] commenting 
  - [x] One liners `// Hello comment`
  - [x] multi line comments `/* Hello comment */`
- [x] arrays
  - [x] initializing int `x[4];`
  - [x] initializing with initializers `int[4] = {0, 1, 2, 3};]`
  - [x] compiler counting initializing `int[] = {1, 2, 3};`
  - [x] using arrays as normal pointers `print(x);`
  - [x] dereferencing arrays `print(*x);`
  - [x] setting arrays `x[0] = 25;` 
  - [x] using arrays `print(x[0]);`
  - [x] multidimensional `argv[0][1]`
- [x] chars `'c'`
- [x] strings `"hello world"`
- [x] ... operator `int printf(char *, ...)`
- [x] [24/4/2020] FIRST GOAL REACHED printf("%s\n", "hello world");
- [x] small cleanup
- [x] [25/4/2020] typedefs
- [x] [26/4/2020] parenthesis in binary operations
- [x] [26/4/2020] casing types
- [x] structs
  - [x] casting structs
  - [x] forward declaring structs
  - [x] declaring structs
    - [x] array initializers
    - [x] . operator initializer
    - [x] packing structs correctly
  - [x] accessing structs
    - [x] . operator access
      - [x] access value with &
    - [x] -> operator access
  - [x] pushing structs
  - [x] assigning structs
    - [x] . assign
    - [x] -> assign
    - [x] normal assign
  - [x] returning structs
    - [x] more than 8 bytes
    - [x] between 0 - 8     GCC DOES NOT DO THIS SO I WON'T DO IT EITHER
  - [x] dereferencing structs
- [x] forward declaration of functions does not interfere with function definitions
- [ ] extern keyword
  - [x] whitespace implemented
  - [-] actually makes a difference
- [x] __attributes__
  - [x] whitespace implemented
  - [ ] ((packed))
  - [ ] ((__nothrow__ )) just treat this as whitespace actually
- [x] restrict and __restrict
  - [x] whitespace implemented
  - [ ] actually makes a difference
- [ ] Unions
  - [ ] Declaration
    - [ ] with tag
    - [ ] anonymous
  - [ ] assignment 
  - [ ] initialising
    - [ ] . operator initialising
    - [ ] array initialiser
- [ ] static keyword
- [x] sizeof keyword
  - [x] currently working but we need to port the whole compiler to bytes and not bits
  - [ ] needs to accept variables too
  - [ ] perfectly working 
- [x] enums 
  - [x] header implemented but not currently usable
  - [x] perfectly working
- [ ] typedef function types
- [ ] __asm__
  - [x] whitespace
  - [ ] perfectly working
- [ ] __attribute__
  - [x] whitespace
  - [ ] perfectly working
- [ ] preprocessor
  - [x] compiler accepts preprocessor input
  - [ ] own preprocessor
- [ ] include files
  - [x] stdarg
  - [x] stddef
- [ ] Scalair initializers are allowed to be inclosed in braces for some reason
- [x][3/5/2020 1:52 AM] SECOND GOAL REACHED printf("hello world") with #include <stdio.h>  
- [ ] major cleanup and refractor
- [ ] ++ --
- [ ] ~ ! ^ & | << >>
- [ ] single statement control flow parsing eg. if (x) print(x); (no brackets))
- [ ] bit fields
- [ ] Character constants like '\x41' are not being parsed properly
- [ ] enums
- [ ] floats
- [ ] doubles
- [ ] preprocessor should support stuff like __file__ __function__ __line__ etc
- [ ] goto and labels
- [ ] switch flow 
- [ ] do while flow
- [ ] control flow keywords (break, continue, case, default, ...)
- [ ] asm("asmcode")
- [ ] const volatile
- [ ] auto, register, restrict
- [ ] we're getting like, reaalyyy close now

# is not going to be implemented
- tentative declarations
- probably register and auto keyword

# Dangers

whenever you use g_symtable to get a symbol remember
that it is based around vectors and that if you use m_scanner.scan()
it could move the table and trash your pointers, so basically
don't use pointers to vectors