#include <stdio.h>

int main()
{
    int x = 5;
    
    if (x)
        printf("x is not 0\n");
    else
        printf("x is 0\n");
    
    x = 0;

    if (x)
        printf("x is not 0\n");
    else
        printf("x is 0\n");
    
    
}