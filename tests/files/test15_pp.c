int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

unsigned int y = 5;

void main()
{
    unsigned int x = 5;
    
    print(x);
    x = &x;
    print(x);
    print(&y);
}
