#pragma once

#include <core.h>

class MemorySpot
{
private:
    bool m_isInit = false;
    int m_memId;
    int m_referencing = -1;
    list <int> m_referencedBy;
    string m_lastName;

public:
    MemorySpot();
    MemorySpot(int memId);
    MemorySpot(MemorySpot *ms);
    void setName(string name);
    void addReferencingTo(MemorySpot *ms);
    void addReferencedBy(int id);
    void destroy(string s);
    void stopBeingReferencedBy(int id);
    void stopReferencing(int id);
    
    string name() { return m_lastName; }
    int references() { return m_referencing; }
    int memId() { return m_memId; }
    int isInit() { return m_isInit; }
    int setMemId(int memId) { m_memId = memId; }
    int setIsInit(bool isInit) { m_isInit = isInit; }
};

class MemoryTable
{
private:
    list <MemorySpot*> m_table;
    int m_memIDCounter = 0;
    
public:
    MemoryTable();
    int addMemorySpot(MemorySpot &ms);
    int removeMemorySpot(int memId);
    MemorySpot *findMemorySpot(int memId);
};

extern MemoryTable g_memTable;