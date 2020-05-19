int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}

void main () {
    print (8+3*5);
    print(50/10-2);
}
