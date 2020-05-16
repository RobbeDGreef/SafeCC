int printf(char *, ...);

struct test
{
    int x;
    int y;
};

struct test func()
{
    struct test x;
    x.x = 12;
    x.y = 25;
    return x;
}

int main()
{
    struct test x = func();
    //x = func();
    //x.x = func().y;
    //x = func();
    printf("x: %i %i\n", x.x, x.y);
}