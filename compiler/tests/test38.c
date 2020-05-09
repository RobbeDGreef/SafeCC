#include <stdio.h>

int main()
{
    char x = 1;
    int y = 4;
    
    if (x && y)
    {
        printf("X is %i and y is %i\n", x, y);
    }
    else
    {
        printf("x or y is 0\n");
    }
    
    int *ptr = &y;
    
    if (ptr && *ptr == 4)
    {
        printf("ptr: %i\n",*ptr);
    }
    else
    {
        printf("yay no seg fault %x\n", ptr);
    }

    for (int i = 0; i <= 10; i++)
    {
        printf("took me a while: %i\n", i);
    }

    int a = 10;
    if (a < 11)
    {
        printf("a: %i\n", a);
    }
    
    int b = 0;
    int c = 1;
    
    if (b || c)
    {
        printf("one or the other\n");
    }
}