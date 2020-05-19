int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main ()

{
    int x;
    x = 7 <= 9;             print(x);
    x = 2 > 0;              print(x);
    x = 123456 < 654321;    print(x);
    x = 1 == 1;             print(x);
    x = 1 != 1;             print(x);
}
