int printf(char *, ...);

int test(void *x)
{
    printf(x);
}

int main()
{
    char s[] = "Hello test";
    
    test(s);
}