int printf(char *fmt, ...);

int x(int y)
{
    printf("%x\n", y);
}

int main()
{
    printf("%s\n", "Hello world :)");
    x(50);
}