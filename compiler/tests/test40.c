#include <stdio.h>

int main()
{
    int x = 5;
    int y = 4;
    int z = 1;
    
    if ( (x == 5 && y == 4) && (z == 1))
    {
        printf("Yes\n");
    }
    else
    {
        printf("no\n");
    }
    
    if (z)
        printf("z is not 0\n");
    
    int a = 0;
    if (!a)
        printf("a is 0\n");
}