int printf(char *, ...);

typedef int *iamatype;
typedef iamatype iamtype2;

int main()
{
    printf("Okay let's do this\n");
    
    int y = -5;
    iamtype2 x = &y;
    
    printf("Value: %i\n", *x);
}