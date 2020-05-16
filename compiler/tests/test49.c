//#include <stdio.h>
int printf(char *, ...);

int *test()
{
    int *p2;
    int c2 = 5;
    p2 = &c2;
    return p2;
}

int main()
{
    int *p = 0;
    
    if (p == 0)
    {
        int c = 4;
        p = &c;
    }
    
    printf("Dangling pointer holds: %i\n", *p);
    
    p = test();
    
    printf("Dangling pointer holds: %i\n", *p);
}