#pragma once

#include <core.h>
#include <errorhandler.h>

class MemorySpot
{
public:
    enum MemLoc
    {
        STATIC = 1,
        HEAP
    };
    

private:
    bool m_isInit = false;
    bool m_isNullInit = false;
    int m_memId;
    int m_referencing = -1;
    int m_memLoc = MemLoc::STATIC;
    list <int> m_referencedBy;
    string m_lastName;
    int m_accessGarbage = false;
    
    string m_destroyedMessage;
    ErrorInfo m_destroyedErrInfo;

public:
    MemorySpot();
    MemorySpot(int memId);
    MemorySpot(MemorySpot *ms);
    MemorySpot(string s);
    void setName(string name);
    void addReferencingTo(MemorySpot *ms);
    void addReferencingTo(MemorySpot *ms, int op);
    void addReferencedBy(int id);
    bool destroy(string s, bool pm=false);
    bool _destroyHeap(string s);
    void destroyedMessage();
    void stopBeingReferencedBy(int id);
    void stopReferencing();
    void copy(MemorySpot *ms);
    bool tryToUse(int op);
    
    string name() { return m_lastName; }
    int references() { return m_referencing; }
    int accessingGarbage() { return m_accessGarbage; }
    int setAccessGarbage(bool ag) { m_accessGarbage = ag; }
    int memId() { return m_memId; }
    int isInit() { return m_isInit; }
    int setMemId(int memId) { m_memId = memId; }
    int setIsInit(bool isInit) { m_isInit = isInit; }
    int setMemLoc(int loc) { m_memLoc = loc; }
};

class MemoryTable
{
private:
    list <MemorySpot*> m_table;
    int m_memIDCounter = 0;
    
public:
    MemoryTable();
    int addMemorySpot(MemorySpot *ms);
    int removeMemorySpot(int memId);
    MemorySpot *findMemorySpot(int memId);
};

extern MemoryTable g_memTable;