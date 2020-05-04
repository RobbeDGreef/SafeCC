int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main()
{
    int x = 0;
    char *y = &x;
    
    print(x);
    print(y);
    print(x);
    print(*y);
    print(y+1);
    print(*y+1);
    print(*&x);
}
