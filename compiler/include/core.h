#pragma once

#include <config.h>

/* C includes */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* C++ includes */
#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <list>

using namespace std;

/* Calc size of array */
#define SIZE(x) sizeof(x) / sizeof(x[0])

int getFullbits(int x);

#ifdef MODE_DEBUG
void debughandler(int sig);
#define DEBUG(x)  std::cout << "\u001b[33;1mDEBUG\u001b[0m: " << x << "\n";
#define DEBUGB(x) std::cout << "\u001b[36;1mDEBUG\u001b[0m: " << x << "\n";
#define DEBUGR(x) std::cout << "\u001b[31;1mDEBUG\u001b[0m: " << x << "\n";
#define DEBUGW(x) std::cout << "\u001b[37;1mDEBUG\u001b[0m: " << x << "\n";

#include <execinfo.h>
#include <signal.h>
inline void __attribute__((always_inline)) print_caller()
{
    void *arr[2];
    backtrace_symbols_fd(arr, backtrace(arr, 2), 2);
}

#define PRINT_CALLER() print_caller()

#else
#define DEBUG(x)
#define DEBUGB(x)
#define DEBUGR(x)
#define DEBUGW(x)
#define PRINT_CALLER()
#define debughandler(x)
#endif

#define RELEASE_PRINT(x) std::cout << x << "\n";

template <class T> int hasItem(vector<T> &items, T item)
{
    if (std::find(items.begin(), items.end(), item) != items.end())
        return 1;
    return 0;
}

#ifndef SOURCE_DIR
#error No source directory was specified when building

// This is just to make vscode happy
#define SOURCE_DIR
#endif
