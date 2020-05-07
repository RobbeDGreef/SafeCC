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
void debughandler(int sig);

#define MODE_DEBUG 

#ifdef MODE_DEBUG
#define DEBUG(x)  std::cout << "\u001b[33;1mDEBUG\u001b[0m: " << x << "\n";
#define DEBUGB(x) std::cout << "\u001b[36;1mDEBUG\u001b[0m: " << x << "\n";
#define DEBUGR(x) std::cout << "\u001b[31;1mDEBUG\u001b[0m: " << x << "\n";
#define DEBUGW(x) std::cout << "\u001b[37;1mDEBUG\u001b[0m: " << x << "\n";
#else
#define DEBUG(x)
#endif

template <class T> int hasItem(vector<T> &items, T item)
{
    if (std::find(items.begin(), items.end(), item) != items.end())
        return 1;
    return 0;
}