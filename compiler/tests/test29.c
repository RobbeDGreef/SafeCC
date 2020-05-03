int printf(char *, ...);

struct test
{
    int x;
    int y;
};

int testfunc(struct test *test)
{
    test->x = 1;
    test->y = 2;
    printf("loc: %x x: %x y: %x\n", test, &test->x, &test->y);
}

int main()
{
    struct test test = {.x = 25, .y = 30};
    
    printf("x: %i, y: %i\n", test.x, test.y);
    testfunc(&test);
    printf("x: %i, y: %i\n", test.x, test.y);
}