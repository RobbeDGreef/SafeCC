#include <stdio.h>

struct hello
{
    int x;
    int y;
};

int test(struct hello x)
{
    printf("int x: %i\n", x.y);
}

int main()
{
    int z = 1;
    for (int i = 0; i < 5; i++)
    {
        struct hello x;
        x.x = 78;
        x.y = 79;
        test(x);
    }
    
    {
        int *y = &z;
        printf("loading: %i\n", *y);
    }
}