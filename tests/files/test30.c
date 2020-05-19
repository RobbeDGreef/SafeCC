int printf(char *, ...);

struct test
{
    int x;
    int y;
};

int func(struct test x)
{
    printf("Content: %i %i\n", x.x, x.y);
}

int main()
{
    struct test x = {.x = 25, .y = 30};
    
    struct test y = x;
    
    y.x = 26;
    
    printf("%i %i\n", x.x, x.y);
    printf("%i %i\n", y.x, y.y);

    
    struct test *z = &x;
    
    struct test a = *z;
    
    printf("%i %i %i %i\n", z->x, z->y, a.x, a.y);
    func(x);
    func(y);
    func(a);
    func(*z);
}

