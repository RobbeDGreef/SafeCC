#pragma once

#include <core.h>
#include <types.h>
#include <attributes.h>

int tokenToSize(int tok);

/// @note: maybe implement the symboltable with a hashmap
struct Symbol
{
    string name;  // Name of the symbol
    int    value; // Value of the symbol, could be
                  // the integer value if symType is set to VARIABLE
                  // but can also be array size if symType is ARRAY
    int used;
    int defined;

    int symType; // The type of symbol (see SymbolTable::SymTypes)

    vector<string> inits; // Initializers for arrays and structs (used if
                          // global)
    Type varType;  // Function return type or variable type
    int         stackLoc; // The stack location to the start of the variable
                          // (used to calc loc on the stack)

    int localVarAmount; // Holds the local variable amount (if function)
    int returnLabelId;  // The generator uses this variable to check
                        // whether the function already has a label at the
                        // return statement

    vector<Type> arguments; // If this symbol is a function
                                   // it needs argument types
    int variableArg;

    int storageClass;
    vector<Attribute> attributes;
};

class Scope : public vector<Symbol>
{
  private:
    int m_index;

  public:
    Scope() { m_index = 0; }
    Scope(int index) { m_index = index; }
    int index() { return m_index; }
};

class SymbolTable
{
  private:
    vector<Scope *> m_scopeList;
    int             m_currentFunctionIndex = -1;
    int             m_stringCount          = 0;

    int             m_staticVariableOffset = 0;
    Scope           m_staticVariables;
    vector<Scope *> m_allScopes;

  public:
    enum SymTypes
    {
        FUNCTION,
        ARGUMENT,
        VARIABLE,
        IMPLICIT,
        LABEL
    };

    enum StorageClass
    {
        AUTO = 0,
        STATIC,
        REGISTER,
        EXTERN
    };

  private:
    int newSymbol();
    int _addVariable(Symbol sym);

  public:
    SymbolTable();
    ~SymbolTable();

    /* HAS to be called everytime we parse a new function */
    void changeCurFunc(int func);
    int  currentFuncIdx();

    struct Scope *_createScope();
    /* These will be called everytime a new scope is entered of left */
    int  newScope();
    bool isCurrentScopeGlobal();
    int  popScope(bool semantics=true, bool functionEnd=false);

    vector<Symbol> getGlobalTable();
    vector<Symbol> getStaticTable();

    int            findSymbol(string);
    int            pushScopeById(int id);
    int            findInCurrentScope(string s);
    Symbol *getSymbol(int index);
    int addSymbol(string symbol, int val, int symType, Type varType,
                  int storageClass);
    /* To be able to pass NULL as vartype */
    int           addSymbol(string sym, int val, int symType, int varType);
    int           addSymbol(string sym, int val, int symType, int varType,
                            int storageClass);
    int           pushSymbol(Symbol);
    int           addString(string val);
    Symbol createSymbol(string sym, int val, int symType,
                               Type varType, int storageClass);
    int           addToFunction(Symbol s);
};

/* Global symtable variable */
extern SymbolTable g_symtable;