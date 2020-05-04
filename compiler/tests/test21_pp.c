int printf(char *, ...);
void print(int x)
{
    printf("%i\n", x);
}


int main()
{
    char t[] = {1, 2, 3};
    
    char c = 'x';
    print(c);
    
    c = 500;
    print(c);

    char *str = "Hello world?\n";
    printf(str);
    print(str[3]);
    print(*str);
    
    char str2[] = "Hello world2?\n";
    printf(str2);
    print(str2[3]);
    print(*str2);
    
}
