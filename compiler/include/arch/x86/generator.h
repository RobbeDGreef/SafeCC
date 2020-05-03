#pragma once

#include <core.h>
#include <generator.h>

#define EAX     0
#define EBX     1
#define ECX     2
#define EDX     3

#define MEMACCESS(symbol)   "[" + symbol + "]"
#define LABEL(label)        ".L" + to_string(label)
#define GETREG(r)           m_registers[m_usedRegisters[r]-1][r]
#define SPECIFYSIZE(r)      m_sizeSpecifiers[r-1]
class GeneratorX86: public Generator
{

private:
    string m_dwordRegisters[4]  = {"eax", "ebx", "ecx", "edx"};
    string m_wordRegisters[4]   = {"ax", "bx", "cx", "dx"};
    string m_loByteRegisters[4] = {"al", "bl", "cl", "dl"};
    string m_hiByteRegisters[4] = {"ah", "bh", "ch", "dh"};
    
    string m_sizeSpecifiers[4]  = {"dword", "word", "byte", "byte"};
    string *m_registers[4] = {m_dwordRegisters, m_wordRegisters,
                              m_hiByteRegisters, m_loByteRegisters};
    
    /* These are 0 if unused, otherwise indexes (-1) to m_registers */
    int m_usedRegisters[4];

    string m_initDataSize[4] = {"db", "dw", "dd", "dq"};

protected:

    int genLoad(int val, int size);
    int genAdd(int reg1, int reg2);
    int genSub(int reg1, int reg2);
    int genMul(int reg1, int reg2);
    int genDiv(int reg1, int reg2);

    int genLoadVariable(int symbolidx);
    int genStoreValue(int reg, int memloc, struct Type t);

    int genCompareJump(int op, int reg1, int reg2, int label);
    int genCompareSet(int op, int reg1, int reg2);
    
    int genJump(int label);
    int genLabel(int label);
    int genWidenRegister(int reg, int oldsize, int newsize, bool isSigned);
    int genPushArgument(int reg, int argindex);
    int genFunctionCall(int symbolidx, int parameters);
    int genReturnJump(int reg, int funcIdx);
    int genLoadLocation(int symbolidx);
    int genPtrAccess(int reg, int size);
    int genDirectMemLoad(int offset, int symbol, int reg, int size);
    int genNegate(int reg);
    int genAccessStruct(int symbol, int idx);

    void freeReg(int reg);
    int allocReg();
    int allocReg(int r);
    void freeAllReg();

    int move(string instr, string source_reg, string dest_reg);
    int checkRegisters();

public:
    GeneratorX86(string);
    int genDataSection();
    int genExternSection();

    int genFunctionPreamble(int funcInx);
    int genFunctionPostamble(int funcIdx);

};