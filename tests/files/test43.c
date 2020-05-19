#include <stdio.h>

// Some beautifull spaghetti code for you
int main()
{
    int flag = 0;
label1:;
    if (flag == 1)
        goto label2;
    printf("Testing\n");
 
    flag = 1;   
    goto label1;

label2:;
}