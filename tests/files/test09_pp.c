int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main ()
{
    int i;
    for (i = 0; i < 10; i = i+1)
    {
        print(i);
    }
}
