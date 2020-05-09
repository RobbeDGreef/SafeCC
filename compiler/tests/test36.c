#include <stdio.h>

int main()
{
    int x = 0xFF01 & 0xFF;
    printf("x: %x\n", x);
    
    x = 0b10 | 0b1;
    printf("x: %x\n", x);
    
    x = 0b110 ^ 0b100;
    printf("x: %x\n", x);
}