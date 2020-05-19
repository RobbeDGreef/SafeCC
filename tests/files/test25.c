int printf(char *, ...);

void theIntPrinter(int x)
{
    printf((char*) "%i\n", x);
}

int main()
{
    int x[] = {25, 5};
    
    int *y = x;
    
    unsigned int ptr = (unsigned int) y;
    theIntPrinter( ((int*)ptr)[0] );
}