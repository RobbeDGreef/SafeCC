#include <stdio.h>

int main()
{
    int x = 0;
    int y = 1;
    int z = 5;
    
    if (y && z)
        printf("y and z\n");
    else if (!x)
        printf("Only x\n");
    else
        printf("other\n");
}