int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main ()
{
    int x = 5;
    int y = 1;

    int z;
    z = x + y;
    int i = 15;
    while (i >= z)
    {
        print(i);
        i = i - 1;
    }

    print(0);
}
