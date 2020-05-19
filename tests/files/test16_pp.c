int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}


void main()
{
    int x = 0xFF;
    print(x);
    print(-0x10bC);
    x = 50;
    print(x);
    
    x = 0b1001;
    print(x);
    
    x = 0717;
    print(x);
    
    print(0564 + 0x87A + 55);
}
