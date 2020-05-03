int print(int);

void main()
{
    int y = 5;
    int *x = &y;
    print(y);
    *x = 3;
    print(y);
    print(x);
    x = x + 1;
    print(x);
}