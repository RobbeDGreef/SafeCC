int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}



void main
 ()


{
    int x = 0;
    
    while (x < 10)
    {
        print(x);
        x = x + 1;
    }
}
