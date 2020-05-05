int printf(char *, ...);
void print(int x)
{
    printf("%x\n", x);
}


char glob[] = {0x1, 0x2};
char glob2[] = {0x2, 0x3, 0b1101};

int main()
{
    int x[4] = {0, 0, 0, 0};
    int z = 50;
    int y[2] = {456, 654};
    int a;
    char b[3] = {258, 5};
    int c;
    
    print(x);
    print(*x);
    print(y[1] + 8);
    
    print(x[1]);
    int i = 1;
    x[i * 0 + 1] = 25;

    print(&a);
    print(&b);
    print(&c);

    print(x[2]);
    int *h = x + 1;
    print(h);
    print(*h);
    print(x[1]);
    print(y[1]);

    print(x[2] + y[1]);
    
    char c1 = 0x41;
    char c2 = 0x42;
    char c3 = 0x43;
    char c4 = 0x44;
    char *multi[4] = {&c1, &c2, &c3, &c4};
    
    print(multi[1]);
    
    print(multi[3][0]);

    print(glob[1]);
     
    return 0;
}
