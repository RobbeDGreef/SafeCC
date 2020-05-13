#include <stdio.h>

int main()
{
    for (int i = 0; i < 5; i++)
    {
        switch (i)
        {
            case 2:
                printf("Two\n");
                break;
            
            case 4:
                printf("For\n");
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