int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main()
{
    print(3 + 8 / 4 * 2 - 3);
}
