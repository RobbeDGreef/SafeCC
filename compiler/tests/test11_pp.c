int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}


void main()
{
    char x = 5;
    char y = 266;
    int z = 266;

    print(x + y + z);
}
