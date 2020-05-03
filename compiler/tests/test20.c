int print(int x);

int main(int argc, char **argv)
{
    print(argc);
    
    int i = 0;
    int c = argv[0][0];
    while (c != 0)
    {
        print(c);
        c = argv[1][i];
        i = i + 1;
    }
    
}