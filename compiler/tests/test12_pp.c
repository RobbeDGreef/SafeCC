int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main()
{
    char c = 5;
    int x = 500;
    c = c;
    print(c);
}
