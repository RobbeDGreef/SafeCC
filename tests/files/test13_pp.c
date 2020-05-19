int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

int z = 13;

void main()
{
    int x = 5;
    char y;
    y = 1;

    x = y + x;

    print(x+y+z);
}
