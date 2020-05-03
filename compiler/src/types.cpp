#include <ast.h>
#include <core.h>
#include <errorhandler.h>
#include <token.h>
#include <types.h>

struct Type g_emptyType = {.primType = 0,
                           .isSigned = false,
                           .size     = 0,
                           .ptrDepth = 0,
                           .name     = NULL,
                           .typeType = 0,
                           .isArray  = false};
struct Type g_intType   = {.primType = PrimitiveTypes::INT,
                         .isSigned = true,
                         .size     = DWORD,
                         .ptrDepth = 0,
                         .name     = NULL,
                         .typeType = TypeTypes::VARIABLE,
                         .isArray  = false};
struct Type g_strType   = {.primType = PrimitiveTypes::CHAR,
                         .isSigned = true,
                         .size     = DWORD,
                         .ptrDepth = 1,
                         .name     = NULL,
                         .typeType = TypeTypes::VARIABLE,
                         .isArray  = true};

struct Type g_defaultType = {.primType = PrimitiveTypes::INT,
                             .isSigned = true,
                             .size     = DWORD,
                             .ptrDepth = 0,
                             .name     = NULL,
                             .typeType = TypeTypes::VARIABLE,
                             .isArray  = false};

struct Type g_ptrType = {.primType = PrimitiveTypes::INT,
                         .isSigned = true,
                         .size     = DWORD,
                         .ptrDepth = 0,
                         .name     = NULL,
                         .typeType = TypeTypes::VARIABLE,
                         .isArray  = false};

int g_regSize     = DWORD;
int g_defaultSize = DWORD;
int g_ptrSize     = DWORD;

string typeString(struct Type *t)
{
    string ret = "";
    if (t->typeType == TypeTypes::STRUCT)
    {
        ret += "struct " + *t->name;
    }
    else
    {
        ret += t->isSigned ? "signed " : "unsigned ";
        ret += typeNames[t->primType];
    }
    if (t->ptrDepth)
        ret += " " + string(t->ptrDepth, '*');

    return ret;
}

int typeToSize(int type)
{
    switch (type)
    {
    case PrimitiveTypes::CHAR:
        return CHAR_SIZE;
    case PrimitiveTypes::SHORT:
        return SHORT_SIZE;
    case PrimitiveTypes::INT:
        return INT_SIZE;
    case PrimitiveTypes::LONG:
        return LONG_SIZE;
    }
    return -1;
}

int _tokenToType(int tok)
{
    return (tok - Token::Tokens::VOID) + 1;
}

int findType(vector<int> &tokens)
{
    for (int t : tokens)
    {
        switch (t)
        {
        case Token::Tokens::VOID:
            return PrimitiveTypes::VOID;
        case Token::Tokens::CHAR:
            return PrimitiveTypes::CHAR;
        case Token::Tokens::SHORT:
            return PrimitiveTypes::SHORT;
        case Token::Tokens::INT:
            return PrimitiveTypes::INT;
        case Token::Tokens::LONG:
            return PrimitiveTypes::LONG;
        default:
            err.fatal("Compiler only supports a type at the end of a type state"
                      "ment (for example 'int signed' should be 'signed int )");
        }
    }
}

struct Type tokenToType(vector<int> &tokens)
{
    if (tokens.size() == 0)
        err.fatal("No type was specified");

    /* Variables are defaultly signed */
    bool sign = true;
    if (hasItem<int>(tokens, Token::Tokens::UNSIGNED))
        sign = false;

    if (hasItem<int>(tokens, Token::Tokens::SIGNED))
        sign = true;

    /* Since the last token must be the variable type itself */
    // (for now)

    struct Type t;
    t.ptrDepth = count(tokens.begin(), tokens.end(), Token::Tokens::STAR);
    t.primType = _tokenToType(tokens[(tokens.size() - 1) - t.ptrDepth]);
    t.isSigned = sign;
    t.typeType = TypeTypes::VARIABLE;
    t.isArray = false;
    if (t.ptrDepth)
        t.size = DWORD;
    else
        t.size = typeToSize(t.primType);
    return t;
}

static int sameType(struct Type l, struct Type r)
{
    if (l.primType == r.primType)
    {
        if (l.isSigned != r.isSigned)
            err.conversionWarning(&l, &r);
        return 1;
    }

    return 0;
}

/* @todo: Throw warnings here maybe ? */
struct ast_node *typeCompatible(struct ast_node *left, struct ast_node *right,
                                bool onlyright)
{
    if ((left->type.typeType == TypeTypes::STRUCT &&
        left->type.typeType != right->type.typeType) ||
        left->type.typeType == TypeTypes::UNION &&
        left->type.typeType != right->type.typeType)
        err.typeConversionError(&left->type, &right->type);

    else if (left->type.typeType == TypeTypes::STRUCT &&
             !left->type.name->compare(*right->type.name))
        return 0;
    else if (left->type.typeType == TypeTypes::STRUCT)
        err.typeConversionError(&left->type, &right->type);

    /* Void is never compatible */
    if (((left->type.primType == PrimitiveTypes::VOID) ||
         (right->type.primType == PrimitiveTypes::VOID)) &&
        left->type.typeType == TypeTypes::VARIABLE && !left->type.ptrDepth)
        err.fatal("Typeerror: void type is not ignored as it ought to be");

    /* Same type is always compatible */
    if (left->type.size == right->type.size)
    {

        if (!left->type.ptrDepth && right->type.ptrDepth)
        {
            err.ptrConversionWarning(&left->type, &right->type);
        }
        else if (left->type.ptrDepth && !right->type.ptrDepth)
        {
            err.ptrConversionWarning(&right->type, &left->type);
        }

        if (left->type.isSigned != right->type.isSigned)
            err.conversionWarning(&left->type, &right->type);

        return 0;
    }

    /* Wider on the left, will widen but is compatible */
    if (left->type.size > right->type.size)
    {
        if (left->type.isSigned != right->type.isSigned)
            err.conversionWarning(&left->type, &right->type);

        if (right->operation == AST::Types::INTLIT)
        {
            /* just scale the size up in the type var */
            right->type.primType = left->type.primType;
            right->type.size     = left->type.size;
        }
        else
            return mkAstUnary(AST::WIDEN, right, right->type.size, left->type,
                              right->line, right->c);
    }

    /* Wider on the right, cannot always widen */
    else if (right->type.size > left->type.size)
    {
        if (onlyright)
            err.typeConversionError(&right->type, &left->type);

        if (left->type.isSigned != right->type.isSigned)
            err.conversionWarning(&left->type, &right->type);

        if (left->operation == AST::Types::INTLIT)
        {
            /* just scale the size up in the type var */
            left->type.primType = right->type.primType;
            left->type.size     = right->type.size;
            return 0;
        }
        else
            return mkAstUnary(AST::WIDEN, left, left->type.size, right->type,
                              right->line, right->c);
    }

    /* anything else fits ?*/
    return 0;
}

int typeFits(struct Type *type, int value)
{
    bool sign = type->isSigned;

    if (type->primType == 0)
        return 1;

    /**
     * Because undefined behaviour is spooky, let's just split
     * the signed and unsigned comparisons up
     */

    if (sign)
    {
        signed int min = -(getFullbits(type->size - 1));
        signed int max = getFullbits(type->size - 1);

        if (value >= min && value <= max)
            return 1;
        else
            return 0;
    }
    else
    {
        unsigned int min = 0;
        unsigned int max = (unsigned int)getFullbits(type->size);

        DEBUG("MIN: " << min << " MAX: " << max << "VALUE: " << value);
        if ((unsigned int)value >= min && (unsigned int)value <= max)
            return 1;
        else
            return 0;
    }
}

int truncateOverflow(struct Type type, int value)
{
    return ((unsigned int)value) & getFullbits(type.size);
}

struct Type guessType(int val, bool issigned)
{
    struct Type ret;
    ret.isSigned = issigned;
    ret.ptrDepth = 0;
    /**
     * Because undefined behaviour is spooky, let's just split
     * the signed and unsigned comparisons up
     */
    if (issigned)
    {
        if (val >= -getFullbits(BYTE - 1) && val <= getFullbits(BYTE - 1))
        {
            ret.primType = PrimitiveTypes::CHAR;
            ret.size     = BYTE;
        }
        else if (val >= -getFullbits(WORD - 1) && val <= getFullbits(WORD - 1))
        {
            ret.primType = PrimitiveTypes::CHAR;
            ret.size     = BYTE;
        }
        else if (val >= -getFullbits(DWORD - 1) &&
                 val <= getFullbits(DWORD - 1))
        {
            ret.primType = PrimitiveTypes::CHAR;
            ret.size     = BYTE;
        }
        else
            err.fatal("Number is too large to fit in any of the supported type "
                      "siezs");
    }
    else
    {

        if ((unsigned int)val <= getFullbits(BYTE))
        {
            ret.primType = PrimitiveTypes::CHAR;
            ret.size     = BYTE;
        }
        else if ((unsigned int)val <= getFullbits(WORD))
        {
            ret.primType = PrimitiveTypes::SHORT;
            ret.size     = WORD;
        }
        else if ((unsigned int)val <= getFullbits(DWORD))
        {
            ret.primType = PrimitiveTypes::INT;
            ret.size     = DWORD;
        }
        else
            err.fatal("Number is too large to fit in any of the supported type "
                      "sizes");
    }
    return ret;
}

void dereference(struct Type *ptr)
{
    if (ptr->ptrDepth)
    {
        ptr->ptrDepth--;
        if (!ptr->ptrDepth)
        {
            // Reset size of operand to correct size
            // @todo: struct sizes
            ptr->size = typeToSize(ptr->primType);
        }
    }
}

int equalType(struct Type l, struct Type r)
{
    if (l.primType == r.primType && l.isSigned == r.isSigned &&
        l.size == r.size && l.ptrDepth == r.ptrDepth &&
        l.typeType == r.typeType)
    {
        if (l.typeType == TypeTypes::STRUCT && !l.name->compare(*r.name))
            return 1;
        else if (l.typeType == TypeTypes::STRUCT)
            return 0;

        return 1;
    }
    return 0;
}

struct Type TypeList::getType(string ident)
{
    for (struct Type t : m_namedTypes)
    {
        if ((*t.name)[0] == ident[0] && !ident.compare(*t.name))
            return t;
    }

    return NULLTYPE;
}

void TypeList::addType(struct Type t)
{
    m_namedTypes.push_back(t);
}

void TypeList::replace(string ident, struct Type t)
{
    for (auto i = m_namedTypes.begin(); i != m_namedTypes.end(); i++)
    {
        struct Type &ref(*i);

        if ((*ref.name)[0] == ident[0] && !ident.compare(*ref.name))
        {
            ref = t;
            return;
        }
    }
}

int findStructItem(string item, struct Type t)
{
    int j = 0;
    for (struct StructItem s : t.contents)
    {
        if (s.name[0] == item[0] && !s.name.compare(item))
            return j;

        j++;
    }

    err.unknownStructItem(item, t);
}