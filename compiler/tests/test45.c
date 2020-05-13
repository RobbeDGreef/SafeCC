#include <stdio.h>

int main()
{
    for (int i = 0; i < 10; i++)
    {
        if (i > 5)
            continue;
        switch (i)
        {
            case 2:
                printf("Two\n");
                break;
            
            case 4:
                printf("Four\n");
                break;
            
            default:
                printf("Default: %i\n", i);
                break;
        }
    }
    
    i = 0;
    while (i < 10)
    {
        if (i > 3 && i < 8)
        {
            i++;
            continue;
        }
        
        printf("I: %i\n", i);
        i++;
    }
}