#include <attributes.h>

bool hasAttr(vector<Attribute> attributes, int attr)
{
    for (auto i = attributes.begin(); i != attributes.end(); ++i)
    {
        if ((*i).attribute == attr)
            return true;
    }
    
    return false;
}


Attribute::Attribute(int attr)
{
    attribute = attr;
}

Attribute getAttr(vector<Attribute> attributes, int attr)
{
    for (auto i = attributes.begin(); i != attributes.end(); ++i)
    {
        if ((*i).attribute == attr)
            return *i;
    }
    
    return false;
}