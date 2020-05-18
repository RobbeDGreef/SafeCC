#pragma once

#include <memtable.h>

/* forward declare ast_node to keep this header standalone */
struct ast_node;
struct StructItem;
struct Symbol;

#define BYTE  8
#define WORD  16
#define DWORD 32
#define QWORD 64

static string typeNames[] = {"nulltype", "void", "char",
                             "short",    "int",  "long",
                             "float", "double"};

struct Type
{
    int  primType; // The primitive type
    bool isSigned; // Whether the type is signed
    int  size;     // Variable size (bytes)

    int ptrDepth; // The dimmention of the pointer (0 if not a pointer)

    string *name; // If the type is non primitive (typedef etc) then
                  // the name will be stored here (can be null obviously)
    int typeType; // The type of type (I know what a name)
    bool isArray;

    vector<struct StructItem> contents; // The contents of a type

    bool incomplete; // Is the type incomplete (forward declare etc)
    MemorySpot *memSpot;
};

struct StructItem
{
    Type itemType; // The type of the item
    int         offset;   // The stack offset to this variable
    string      name;
};

#define CHAR_SIZE  (BYTE / 8)
#define SHORT_SIZE (WORD / 8)
#define INT_SIZE   (DWORD / 8)
#define LONG_SIZE  (DWORD / 8)
#define PTR_SIZE (DWORD / 8)
#define LONGLONG_SIZE (QWORD / 8)
#define FLOAT_SIZE (DWORD / 8)
#define DOUBLE_SIZE (DWORD / 8)

#define REGISTERSIZE g_regSize
#define DEFAULTSIZE  g_defaultSize
extern int g_regSize;
extern int g_defaultSize;

#define NULLTYPE    g_emptyType
#define INTTYPE     g_intType
#define PTRTYPE     g_ptrType
#define STRINGPTR   g_strType
#define DEFAULTTYPE g_defaultType
extern Type g_locationSpecifier;
extern Type g_emptyType;
extern Type g_intType;
extern Type g_strType;
extern Type g_defaultType;
extern Type g_ptrType;

enum PrimitiveTypes
{
    VOID = 1,
    CHAR,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE
};

enum TypeTypes
{
    VARIABLE = 1,
    STRUCT,
    UNION,
    ENUM, 
    CONSTANT
};

class TypeList
{
  private:
    // This list will hold named types like structs, unions and typedefs
    list<Type> m_namedTypes;

  public:
    Type getType(string ident);
    void        addType(Type);
    void        replace(string ident, Type t);
};

extern TypeList g_typeList;

int              equalType(Type l, Type r);
string           typeString(Type *t);
Type      tokenToType(vector<int> &tokens);
Type      guessType(int val, bool isSigned);
ast_node *typeCompatible(ast_node *left, ast_node *right,
                                bool onlyright);
int              typeFits(Type *type, int value);
int              truncateOverflow(Type type, int value);
int              typeToSize(int type);
void             dereference(Type *ptr);
int              findStructItem(string item, Type t);

int getArraySize(Symbol *s);
int getTypeSize(Symbol sym);