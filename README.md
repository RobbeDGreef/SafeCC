# SafeCC
SafeCC is a simple C compiler that tries to help the user find as many
bugs at compile time as possible. This includes out-of-scope checking,
memory leakage checking, double free checking and more. This compiler is
very much in its early stages and thus is currently unsuitable for any
'real' development. You are welcome to contribute to this project and help
with what we hope to soon be a full memory safe C compiler.

## Getting started
SafeCC should normally be pretty easy to build with any ordinary 
c++ compiler, so just grab a copy and follow the installation guide below.

### Prerequisites
Since this compiler currently doesn't have its own preprocessor, it will try
to invoke gcc for the preprocessing step. This means that gcc should be in 
your PATH. The same goes for the linking process.

For the assembling step the compiler will try to use nasm so, nasm should
also be in your PATH variable.

### Installing
Just `cd` into the compiler directory and run `./build.sh` to start the build process.

## Running the tests
The tests folder includes a large amount of test files, to test them all at
once run `./tests.sh`. If you want to try one test case individually
run `./test.sh <testName.c>`, where <testName.c> is obviously changed to the
test name you want to try. 

## Features
Currently SafeCC checks for:
- Out-of-scope references
- Use after free bugs
- Wild pointer (use before initialization)
- Double free bugs
- Memory leaks

This is supposed to expand in the future and the current tests are still
in early development so it might not behave as expected

## Contributing
Please read CONTRIBUTING.md for details on our code of conduct, and the process
of submitting pull requests to us.

## License
This project is licensed under GPL3 License - see the LICENSE.md file for details

## Acknowledgements
These other C compilers were definitely a great guide on how to tackle this 
problem
- The subC compiler
- SmallerC compiler

## Resources that deserve a special thank you
- https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-798.pdf
