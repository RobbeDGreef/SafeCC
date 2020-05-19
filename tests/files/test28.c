int printf(char *, ...);

struct test
{
    char p;
    short x;
    short y;
    
    int ix;
    int iy;
    
    char a;
    char b;
    
    int c;
};

int main()
{
    struct test t;
    printf("p: %x\n", &t.p);
    printf("x: %x\n", &t.x);
    printf("y: %x\n", &t.y);
    printf("ix: %x\n", &t.ix);
    printf("iy: %x\n", &t.iy);
    printf("a: %x\n", &t.a);
    printf("b: %x\n", &t.b);
    printf("c: %x\n", &t.c);
}