int printf(char *, ...);

struct hello
{
    int x;
    int y;
    int z;
};

int main()
{
    struct hello y = {.x = 1, .y = 2, .z=3};
    
    struct hello *x = &y;
    
    printf("contents of x: %i %i %i\n", x->x, x->y, x->z);
    printf("Content: %i %i %i\n", y.x, y.y, y.z);
    printf("%x %x\n", x, (&y)->x);
    return 0; 
}