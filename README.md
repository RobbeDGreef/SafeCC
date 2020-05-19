# SafeCC
SafeCC is a simple POC C compiler that tries to help the user find as many
bugs at compile time as possible. This includes out-of-scope checks,
memory leakage tracking, double free checking and more. The compiler is
very much in its early stages and thus is currently unsuitable for any
'real' development. 

## Getting started
SafeCC should normally be pretty easy to build with any ordinary 
c++ compiler, so just grab a copy with

    git clone https://github.com/RobbeDGreef/SafeCC.git

and follow the installation guide below.

### Prerequisites
Since this compiler currently doesn't have its own preprocessor, 
it will try to invoke gcc for the preprocessing step. 
This means that gcc should be in your PATH. The same goes ld for
the linking process and nasm for the assembling step.

SafeCC is build with cmake and make so you need will need those too.

You can install all these by running (on a debian-based system)

    sudo apt-get install build-essentials nasm cmake

### Installing
The installation steps are very simple. We have made it easy for you (and me),
if you just want to build the project quickly run `./build.sh` in the main
directory. If you want some more control read below.

SafeCC is build with cmake and it is good practice to build to project in a 
seperate 'build' directory, so that is what we do here:

    mkdir build
    cd build
    cmake ..
    make all
    mv safecc ../safecc
    cd ..

as you can see this means we have to move the executable out of the build
folder but we can easily do that with the `mv` command.

You can now compile C source files with

    ./safecc -o <outfile> <infiles>

## Running the tests
The tests folder includes a fair amount of test files, to test them all at
once run `./tests/tests.sh`. If you want to try one test case individually
run `./tests/test.sh tests/files/<testName.c>`, where <testName.c> is obviously changed
to the test name you want to try. 

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
Please read CONTRIBUTING.md for details on our code of conduct, and the process of submitting pull requests to us.

## License
This project is licensed under GPL3 License - see the LICENSE.md file for details

## Acknowledgements
These other C compilers were definitely a great guide on how to tackle
this problem
- The subC compiler
- SmallerC compiler

## Resources that deserve a special thank you
- https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-798.pdf
