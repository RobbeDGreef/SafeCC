#include <stdio.h>

int main()
{
    for (int i = 0; i < 5; i++)
    {
        switch(i)
        {
            case 1:
            case 2: 
                printf("2\n");
                printf("Test\n");
                goto endLabel;
            case 4: printf("4\n");
                    goto endLabel;
            
            
            default: printf("Default %i\n", i);
        }
endLabel:;
    }
    
}