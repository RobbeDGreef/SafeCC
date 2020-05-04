int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}


int ImIntermediate(int x, int y)
{
    print(x);
    print(y);
    int z = 1;
    return x * y + z;
    
    print(123456);
}

int voidf()
{
    print (1);
}

int main()
{
    int x = -5;
    int y = 50;
    print(ImIntermediate(ImIntermediate(x,y), 8));
    voidf();
}
