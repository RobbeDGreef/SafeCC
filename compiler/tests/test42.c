#include <stdio.h>

int main()
{
    int x = 1;
    int y = 2;
    int a = 1;
    int z = !(x == 1 && a == 0) || !(y == 2);
    printf("x: %i\n", z);
}