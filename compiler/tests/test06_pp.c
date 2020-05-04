int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main ()
{
    int i = 6;
    if (i < 3) {
        print(i);
    } else {
        print(3);
    }
}
