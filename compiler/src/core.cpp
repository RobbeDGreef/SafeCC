#include <core.h>

#if 0
template <class T>
int hasItem(vector<T> &items, T item)
{
    if (std::find(items.begin(), items.end(), item) != items.end())
        return 1;
    return 0;
}
#endif

int getFullbits(int x)
{
    int r = 0;
    for (int i = 0; i < x; i++)
    {
        r = (r << 1) | 0x1; 
    }
    return r;
}
