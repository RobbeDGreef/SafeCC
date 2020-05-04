int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main ()
{
    for (int i = 5; i > 0; i = i - 1 )
    {
        print(i);
    }
}
