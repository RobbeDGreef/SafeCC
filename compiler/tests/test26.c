int printf(char *, ...);

struct hellostruct
{
    int x;
    int y;
    int z;
};

struct hellostruct2
{
    int x;
    int y;
    int z;
};

int test(int x)
{
    
}

int receive(struct hellostruct x)
{
    printf("%x %x %x", x, x.x);
}

int main()
{
    struct hellostruct x = {40, 50, 60};
    struct hellostruct2 y = {70, 80 ,90};
    struct hellostruct z = {.x = 25, .z = 5, .y = 320, .y = 30};
    
    receive(z);
}