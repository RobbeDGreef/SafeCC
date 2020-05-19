#include <stdio.h>

int main()
{
    int x = ~0xFFFF;
    printf("x: %x\n", x);
    
    x = ~0xFFFF0000;
    printf("x: %x\n", x);
}