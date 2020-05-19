int printf(char *, ...);

int main()
{
    int *p = 0;
    
    if (p == 0)
    {
        int c = 4;
        p = &c;
    }
    
    printf("Dangling pointer holds: %i\n", p[0]);
}