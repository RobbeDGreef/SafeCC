#pragma once

#include <core.h>

enum Attributes
{
    RETURNS_HEAPVAL = 1,
    CLEARS_HEAPVAL,
    META
};

// Very simple attribute class
class Attribute
{
public:
    int attribute;
    vector <int> values;

public:
    Attribute(int attr);
};

bool hasAttr(vector<Attribute> attributes, int attr);
Attribute getAttr(vector<Attribute> attributes, int attr);