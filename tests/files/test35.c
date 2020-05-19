#include <stdio.h>

int main()
{
    int x = 1;
    
    printf("x: %i\n", x++);
    printf("x: %i\n", ++x);
    
    x++;
    
    x = 2 + x;
    
    printf("x: %i\n", x);

    char y[3];

    y[2] = 52;    
    y[1] = y[2] + (char) x;
    
    printf("new: %i\n", y[1]);
    
    int z = 8 % 5;
    printf("z: %i\n", z);
    
    int a = 1 << 2 + 25 / 5;
    
    printf("a: %i\n", a);
    
    int b = y[1] + y[2];
    printf("b: %i %i %i\n", y[1], y[2], b);
}