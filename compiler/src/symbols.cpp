#include <errorhandler.h>
#include <symbols.h>
#include <types.h>

/* Global symbol table */
SymbolTable g_symtable = SymbolTable();

SymbolTable::SymbolTable()
{
    /* The global symbol table is the first scope */
    newScope();
}

SymbolTable::~SymbolTable()
{
    for (vector<struct Symbol> *scope : m_scopeList)
        delete scope;
}

void SymbolTable::newScope()
{
    m_scopeList.push_back(new vector<struct Symbol>);
}

void SymbolTable::pushScope(vector<struct Symbol> *scope)
{
    m_scopeList.push_back(scope);
}

vector<struct Symbol> *SymbolTable::popScope()
{
    vector<struct Symbol> *scope = m_scopeList.back();
    m_scopeList.pop_back();
    return scope;
}

int SymbolTable::findSymbol(string sym)
{
    /* @todo this is a hack man wth */
    for (int table = m_scopeList.size() - 1; table >= 0; table--)
    {
        int i = 0;
        /* @todo: is this even remotely efficient? */
        for (struct Symbol s : *m_scopeList[table])
        {
            if (s.name[0] == sym[0] && !s.name.compare(sym))
                return (i << 8) | (char)table;
            i++;
        }
    }

    return -1;
}

int SymbolTable::findInCurrentScope(string sym)
{
    int i = 0;
    /* @todo: is this even remotely efficient? */
    for (struct Symbol s : *m_scopeList.back())
    {
        if (s.name[0] == sym[0] && !s.name.compare(sym))
            return (i << 8) | (char)(m_scopeList.size() - 1);
        i++;
    }

    return -1;
}

struct Symbol *SymbolTable::getSymbol(int sym)
{
    /* @todo: this really can't be good */
    return &(*m_scopeList[sym & 0xFF])[sym >> 8];
}

int SymbolTable::_addVariable(struct Symbol sym)
{
    if (isCurrentScopeGlobal())
    {
        if (sym.storageClass == StorageClass::AUTO)
            sym.storageClass = StorageClass::STATIC;
        m_scopeList[0]->push_back(sym);
        return ((m_scopeList[0]->size() - 1) << 8) | 0;
    }
    
    int varSize = getTypeSize(sym);
    
    if (sym.storageClass == StorageClass::STATIC)
    {
        sym.stackLoc = m_staticVariableOffset;
        m_staticVariables.push_back(sym);
        m_staticVariableOffset += varSize;
    }

    struct Symbol *func = getSymbol(m_currentFunctionIndex);

    if (sym.symType == SymTypes::IMPLICIT)
    {
        func->localVarAmount += 4;
        return 0;
    }

    if (sym.symType != SymTypes::ARGUMENT && 
        sym.storageClass != StorageClass::STATIC)
    {
        sym.stackLoc = func->localVarAmount;
        if (sym.varType.isArray)
        {
            func->localVarAmount += varSize;
            
            // Making sure the bytealignment stays correct
            if (func->localVarAmount % INT_SIZE)
                func->localVarAmount += INT_SIZE -
                                        (func->localVarAmount % INT_SIZE);
        }
        else if (sym.varType.typeType == TypeTypes::STRUCT &&
                 !sym.varType.ptrDepth)
            func->localVarAmount += varSize;

        else if (sym.varType.typeType == TypeTypes::STRUCT &&
                 sym.varType.ptrDepth)
            func->localVarAmount += INT_SIZE;

        /* Variables get padded up to keep the 4 byte boundry */
        else if (sym.varType.typeType == TypeTypes::VARIABLE)
            func->localVarAmount += INT_SIZE;
    }


    m_scopeList.back()->push_back(sym);
    return ((m_scopeList.back()->size() - 1) << 8) | (m_scopeList.size() - 1);
}

int SymbolTable::addSymbol(string sym, int val, int symType,
                           struct Type varType, int storageClass)
{
    struct Symbol newsym;
    memset(&newsym, 0, sizeof(struct Symbol));

    newsym.name           = sym;
    newsym.value          = val;
    newsym.symType        = symType;
    newsym.varType        = varType;
    newsym.localVarAmount = 0;
    newsym.stackLoc       = 0;
    newsym.returnLabelId  = -1;
    newsym.variableArg    = 0;
    newsym.storageClass = storageClass;
    newsym.defined = false;
    newsym.used = false;

    return _addVariable(newsym);
}

int SymbolTable::addSymbol(string sym, int val, int symType, int varType)
{
    struct Type t;
    memset(&t, 0, sizeof(struct Type));
    return addSymbol(sym, val, symType, t, StorageClass::AUTO);
}

int SymbolTable::addSymbol(string sym, int val, int symType, int varType, int sc)
{
    struct Type t;
    memset(&t, 0, sizeof(struct Type));
    return addSymbol(sym, val, symType, t, sc);
}

vector<struct Symbol> SymbolTable::getGlobalTable()
{
    return *m_scopeList[0];
}

int SymbolTable::pushSymbol(struct Symbol s)
{
    return _addVariable(s);
}

bool SymbolTable::isCurrentScopeGlobal()
{
    if (m_scopeList.size() == 1)
        return true;

    return false;
}

void SymbolTable::changeCurFunc(int func)
{
    m_currentFunctionIndex = func;
}

int SymbolTable::currentFuncIdx()
{
    return m_currentFunctionIndex;
}

int SymbolTable::addString(string str)
{
    struct Symbol s;
    memset(&s, 0, sizeof(struct Symbol));
    s.name         = "S" + to_string(m_stringCount++);
    s.varType      = STRINGPTR;
    s.symType      = SymTypes::VARIABLE;
    s.storageClass = StorageClass::STATIC;

    for (char c : str)
        s.inits.push_back(to_string(c));

    s.inits.push_back("0");
    s.value = s.inits.size();

    // Strings are added in the global scope
    m_scopeList[0]->push_back(s);

    return ((m_scopeList[0]->size() - 1) << 8);
}
