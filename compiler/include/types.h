#pragma once

#include <memtable.h>

/* forward declare ast_node to keep this header standalone */
struct ast_node;

#define BYTE  8
#define WORD  16
#define DWORD 32
#define QWORD 64

static string typeNames[] = {"nulltype", "void", "char",
                             "short",    "int",  "long"};

struct StructItem;

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
    struct Type itemType; // The type of the item
    int         offset;   // The stack offset to this variable
    string      name;
};

#define CHAR_SIZE  (BYTE / 8)
#define SHORT_SIZE (WORD / 8)
#define INT_SIZE   (DWORD / 8)
#define LONG_SIZE  (DWORD / 8)
#define PTR_SIZE (DWORD / 8)
#define LONGLONG_SIZE (QWORD / 8)

#define REGISTERSIZE g_regSize
#define DEFAULTSIZE  g_defaultSize
extern int g_regSize;
extern int g_defaultSize;

#define NULLTYPE    g_emptyType
#define INTTYPE     g_intType
#define PTRTYPE     g_ptrType
#define STRINGPTR   g_strType
#define DEFAULTTYPE g_defaultType
extern struct Type g_locationSpecifier;
extern struct Type g_emptyType;
extern struct Type g_intType;
extern struct Type g_strType;
extern struct Type g_defaultType;
extern struct Type g_ptrType;

enum PrimitiveTypes
{
    VOID = 1,
    CHAR,
    SHORT,
    INT,
    LONG
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
    list<struct Type> m_namedTypes;

  public:
    struct Type getType(string ident);
    void        addType(struct Type);
    void        replace(string ident, struct Type t);
};

extern TypeList g_typeList;

int              equalType(struct Type l, struct Type r);
string           typeString(struct Type *t);
struct Type      tokenToType(vector<int> &tokens);
struct Type      guessType(int val, bool isSigned);
struct ast_node *typeCompatible(struct ast_node *left, struct ast_node *right,
                                bool onlyright);
int              typeFits(struct Type *type, int value);
int              truncateOverflow(struct Type type, int value);
int              typeToSize(int type);
void             dereference(struct Type *ptr);
int              findStructItem(string item, struct Type t);

int getArraySize(struct Symbol *s);
int getTypeSize(struct Symbol sym);