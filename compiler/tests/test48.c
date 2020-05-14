#include <stdio.h>

int main()
{
    int x = 5;
    printf("x: %i\n", x);

    x += 3;
    printf("x: %i\n", x);
    
    x /= 4;
    printf("x: %i\n", x);
    
    x *= 2;
    printf("x: %i\n", x);
    
    x %= 3;
    printf("x: %i\n", x);
    
    x <<= 3;
    printf("x: %i\n", x);
}