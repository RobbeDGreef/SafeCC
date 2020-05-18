#include <stdio.h>
#include <stdlib.h>

//void *alloc(size_t size)
//{
//    return malloc(size);
//}

int main()
{
    int *p;
    {
        int *p2 = malloc(4);
        
        p = p2;
    }
    
    printf("loc: %x\n", p);
    
    int *p3 = malloc(4);
    
    printf("loc of p3: %x\n", p3);
    
    free(p3);
    
    printf("loc of p3: %x\n", p3);
    
    //int *p4 = alloc(50);
}