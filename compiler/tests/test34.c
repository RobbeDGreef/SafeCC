#include <stdio.h>

enum week 
{
    monday = 5, tuesday, wednessday, thursday, friday, saturday, sunday
};

int main()
{
    printf("Enum test: %i %i %i %i\n", monday, tuesday, wednessday, thursday);
    
    enum week week = tuesday;
    enum week test = 5;
    
    printf("week: %i\n", week);
}