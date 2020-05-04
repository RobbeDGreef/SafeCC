int print(unsigned int);

void main()
{
    int x = 0;
    char *y = &x;
    
    print(x);
    print(y);
    print(x);
    print(*y);
    print(y+1);
    print(*y+1);
    print(*&x);
}