int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main ()
{
    int x;
    int y;
    x = -5;
    y = x;
    print (x+y);
}
