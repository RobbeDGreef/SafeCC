#include <ast.h>
#include <errorhandler.h>
#include <memtable.h>
#include <types.h>

MemoryTable g_memTable = MemoryTable();

MemorySpot::MemorySpot()
{
    g_memTable.addMemorySpot(this);
}

MemorySpot::MemorySpot(int memId)
{
    m_memId = memId;
}

MemorySpot::MemorySpot(string s)
{
    g_memTable.addMemorySpot(this);
    m_lastName = s;
}

MemorySpot::MemorySpot(MemorySpot *ms, bool deepCopy)
{
    if (ms && deepCopy)
        *this = *ms;
    
    g_memTable.addMemorySpot(this);
    if (ms)
        addReferencingTo(ms, 0);
}


void MemorySpot::addReferencingTo(MemorySpot *ms)
{
    #if 0
    if (ms->m_memLoc == MemLoc::HEAP)
    {
        m_referencing = ms->memId();
        ms->addReferencedBy(m_memId);
    }
    if (ms->m_memLoc == MemLoc::STATIC)
    {
        if (ms->references() != -1)
        {
        }
    }
    #endif
    
    m_referencing = ms->memId();
    ms->addReferencedBy(m_memId);
}

void MemorySpot::addReferencingTo(MemorySpot *ms, int op)
{
    int ref = -1;
    switch (op)
    {
    case AST::Types::LOADLOCATION:
        ref = ms->memId();
        break;
    case AST::Types::PTRACCESS:
        ref = -1;
        break;
    default:
        if (ms->m_memLoc != MemLoc::HEAP && !ms->accessingGarbage())
            ref = ms->references();
        else
            ref = ms->memId();
    }
    
    if (ref != -1)
        addReferencingTo(g_memTable.findMemorySpot(ref));
    
    ms->checkDestroy();
}

void MemorySpot::setName(string name)
{
    m_lastName = name;
}

void MemorySpot::addReferencedBy(int id)
{
    m_referencedBy.push_back(id);
}

void MemorySpot::stopBeingReferencedBy(int id)
{
    m_referencedBy.remove(id);

    string lastref = g_memTable.findMemorySpot(id)->name();
    if (!m_referencedBy.size() && m_memLoc == MemLoc::HEAP)
        err.memWarn("Memory leak, memory is allocated at the heap but never "
                    "freed\n"
                    "The last reference (" +
                    lastref + ") just went out of scope");
}

void MemorySpot::stopReferencing()
{
    m_referencing = -1;
}

void MemorySpot::copy(MemorySpot *ms)
{
    if (ms)
        *this = *ms;
}

bool MemorySpot::_destroyHeap(string s)
{
    m_destroyedMessage = "Variable " + HL("'" + s + "'") +
                         " was destroyed by a memory deallocator";
    for (int id : m_referencedBy)
    {
        g_memTable.findMemorySpot(id)->m_accessGarbage = true;
    }
    // g_memTable.removeMemorySpot(m_memId);
    return false;
}

void MemorySpot::destroyedMessage()
{
    if (m_referencing != -1)
        g_memTable.findMemorySpot(m_referencing)->destroyedMessage();
    else
    {
        err.loadErrorInfo(m_destroyedErrInfo);
        err.memNotice(m_destroyedMessage);    
    }
}

bool MemorySpot::tryToUse(int op)
{
    MemorySpot *ms = NULL;
    if (m_referencing != -1)
    {
        ms = g_memTable.findMemorySpot(m_referencing);
        DEBUGR("refencing: " << m_referencing << " name: " << ms->name())
    }
    
    switch (op)
    {
    case AST::Types::PTRACCESS:
    
        if (!m_isInit || (ms && !ms->isInit()))
            err.memWarn("Trying to access memory that is not initialised");
        if (m_isNullInit || (ms && ms->isNullInit()))
            err.memWarn("Trying to access null-initialised memory");
        if (m_accessGarbage || (ms && ms->accessingGarbage()))
        {
            if (ms)
                ms->destroyedMessage();
            
            err.memWarn("Trying to access memory that points to garbage");
        }
        break;
    }

    return false;
}

bool MemorySpot::destroy(string s, bool printMessage)
{
    bool ret           = false;
    m_destroyedErrInfo = err.createErrorInfo();

    if (m_memLoc == MemLoc::HEAP)
        return _destroyHeap(s);

    if (m_referencing != -1 && !m_accessGarbage)
    {
        g_memTable.findMemorySpot(m_referencing)->stopBeingReferencedBy(m_memId);
    }

    if (m_referencedBy.size())
    {
        m_destroyedMessage = "Variable " + HL("'" + s + "'") +
                             " went out of scope but was still referenced by:\n";
        
        int i = 0;
        for (int id : m_referencedBy)
        {
            m_destroyedMessage += HL(g_memTable.findMemorySpot(id)->name());
            if (i != m_referencedBy.size() - 1)
                m_destroyedMessage += ", ";
            i++;
        }
        
        // Trying to destory memory object that is still referenced
        if (printMessage)
            err.memWarn(m_destroyedMessage);
        
        // Since this is just a warning lets deref those too
        for (int id : m_referencedBy)
        {
            g_memTable.findMemorySpot(id)->setAccessGarbage(true);
        }
        m_referencedBy.clear();
    }

    // g_memTable.removeMemorySpot(m_memId);
    return ret;
}

MemoryTable::MemoryTable()
{
}

int MemoryTable::addMemorySpot(MemorySpot *ms)
{
    m_table.push_back(ms);
    ms->setMemId(m_memIDCounter++);
}

int MemoryTable::removeMemorySpot(int memId)
{
    for (auto i = m_table.begin(); i != m_table.end(); i++)
    {
        if ((*i)->memId() == memId)
        {
            m_table.erase(i);
            return 0;
        }
    }
    return -1;
}

MemorySpot *MemoryTable::findMemorySpot(int memId)
{
    for (auto i = m_table.begin(); i != m_table.end(); i++)
    {
        if ((*i)->memId() == memId)
        {
            return *i;
        }
    }

    err.fatal("Could not find memory spot in table " + to_string(memId));
}

void MemorySpot::goodValues()
{
    m_isInit = true;
    m_isNullInit = false;
    m_accessGarbage = false;
}

int MemorySpot::checkDestroy()
{
    if (!m_referencedBy.size())
        destroy("");
}

int MemorySpot::references()
{
    return m_referencing;
}