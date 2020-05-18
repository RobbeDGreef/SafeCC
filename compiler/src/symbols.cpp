#include <errorhandler.h>
#include <symbols.h>
#include <types.h>

/* Global symbol table */
SymbolTable g_symtable = SymbolTable();

Scope *SymbolTable::_createScope()
{
    Scope *scope = new Scope(m_allScopes.size());
    m_allScopes.push_back(scope);
    return scope;
}

SymbolTable::SymbolTable()
{
    /* The global symbol table is the first scope */
    newScope();
}

SymbolTable::~SymbolTable()
{
    for (Scope *scope : m_scopeList)
        delete scope;
}

int SymbolTable::newScope()
{
    Scope *scope = _createScope();
    m_scopeList.push_back(scope);
    return scope->index();
}

int SymbolTable::popScope(bool semantics, bool functionEnd)
{
    Scope *scope = m_scopeList.back();
    if (semantics)
    {
        for (Symbol s : *scope)
            if (s.varType.memSpot)
                s.varType.memSpot->setName(s.name);
        
        bool warn = false;
        // Iterate backwards to remove the last allocated memory first
        for (auto i = scope->rbegin(); i != scope->rend(); ++i)
        {
            if ((*i).varType.memSpot)
            {
                if ((*i).varType.memSpot->destroy((*i).name, functionEnd))
                    warn = true;
            }
            //delete s.varType.memSpot;
            (*i).varType.memSpot = NULL;
        }
        
        if (warn && functionEnd)
        {
            string fname = getSymbol(m_currentFunctionIndex)->name;
            err.notice("At the end of function '" + HL(fname) + "'");
        }
    }

    m_scopeList.pop_back();
    return scope->index();
}

int SymbolTable::findSymbol(string sym)
{
    /* @todo this is a hack man wth */
    for (int table = m_scopeList.size() - 1; table >= 0; table--)
    {
        int i = 0;
        /* @todo: is this even remotely efficient? */
        for (Symbol s : *m_scopeList[table])
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
    for (Symbol s : *m_scopeList.back())
    {
        if (s.name[0] == sym[0] && !s.name.compare(sym))
            return (i << 8) | (char)(m_scopeList.size() - 1);
        i++;
    }

    return -1;
}

Symbol *SymbolTable::getSymbol(int sym)
{
    /* @todo: this really can't be good */
    return &(*m_scopeList[sym & 0xFF])[sym >> 8];
}

int SymbolTable::_addVariable(Symbol sym)
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

    Symbol *func = getSymbol(m_currentFunctionIndex);

    if (sym.symType == SymTypes::IMPLICIT)
    {
        func->localVarAmount += 4;
        return 0;
    }

    if (sym.symType != SymTypes::ARGUMENT && sym.symType != SymTypes::LABEL &&
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

int SymbolTable::addToFunction(Symbol s)
{
    if (m_currentFunctionIndex != -1)
    {
        m_scopeList[1]->push_back(s);
        return ((m_scopeList[1]->size() - 1) << 8) | 1;
    }

    return -1;
}

Symbol SymbolTable::createSymbol(string sym, int val, int symType,
                                        Type varType, int storageClass)
{
    Symbol newsym;
    memset(&newsym, 0, sizeof(Symbol));

    newsym.name           = sym;
    newsym.value          = val;
    newsym.symType        = symType;
    newsym.varType        = varType;
    newsym.localVarAmount = 0;
    newsym.stackLoc       = 0;
    newsym.returnLabelId  = -1;
    newsym.variableArg    = 0;
    newsym.storageClass   = storageClass;
    newsym.defined        = false;
    newsym.used           = false;

    return newsym;
}

int SymbolTable::addSymbol(string sym, int val, int symType,
                           Type varType, int storageClass)
{
    return _addVariable(createSymbol(sym, val, symType, varType, storageClass));
}

int SymbolTable::addSymbol(string sym, int val, int symType, int varType)
{
    return addSymbol(sym, val, symType, NULLTYPE, StorageClass::AUTO);
}

int SymbolTable::addSymbol(string sym, int val, int symType, int varType,
                           int sc)
{
    Type t;
    memset(&t, 0, sizeof(Type));
    return addSymbol(sym, val, symType, t, sc);
}

vector<Symbol> SymbolTable::getGlobalTable()
{
    return *m_scopeList[0];
}

int SymbolTable::pushSymbol(Symbol s)
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
    Symbol s;
    memset(&s, 0, sizeof(Symbol));
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

int SymbolTable::pushScopeById(int id)
{
    m_scopeList.push_back(m_allScopes[id]);
    return id;
}