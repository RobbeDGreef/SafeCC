#include <errorhandler.h>
#include <memtable.h>

MemoryTable g_memTable = MemoryTable();

MemorySpot::MemorySpot()
{
    g_memTable.addMemorySpot(*this);
}

MemorySpot::MemorySpot(int memId)
{
    m_memId = memId;
}

MemorySpot::MemorySpot(MemorySpot *ms)
{
    g_memTable.addMemorySpot(*this);
    addReferencingTo(g_memTable.findMemorySpot(ms->references()));
}

void MemorySpot::addReferencingTo(MemorySpot *ms)
{
    m_referencing = ms->memId();
    ms->addReferencedBy(m_memId);
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
}

void MemorySpot::stopReferencing(int id)
{
    m_referencing = -1;
}

void MemorySpot::destroy(string s)
{
    if (m_referencing != -1)
    {
        g_memTable.findMemorySpot(m_referencing)->stopBeingReferencedBy(m_memId);
    }

    if (m_referencedBy.size())
    {
        // Trying to destory memory object that is still referenced

        string names = "";
        int i = 0;
        for (int id : m_referencedBy)
        {
            names += HL(g_memTable.findMemorySpot(id)->name());
            if (i != m_referencedBy.size() - 1)
                names += ", ";
        }

        // @todo warn specific cases
        err.memWarn("'" + s + "' goes out of scope but is still referenced " +
                    to_string(m_referencedBy.size()) + " time(s)\nby: " +
                    names);
        
        // Since this is just a warning lets deref those too
        for (int id : m_referencedBy)
        {
            g_memTable.findMemorySpot(id)->stopReferencing(m_memId);
        }
        m_referencedBy.clear();
    }
}

MemoryTable::MemoryTable()
{
}

int MemoryTable::addMemorySpot(MemorySpot &ms)
{
    m_table.push_back(&ms);
    ms.setMemId(m_memIDCounter++);
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

    err.fatal("Could not find memory spot in table");
}