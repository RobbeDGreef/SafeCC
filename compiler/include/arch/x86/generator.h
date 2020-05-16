#pragma once

#include <core.h>
#include <generator.h>

#define EAX       0
#define EBX       1
#define ECX       2
#define EDX       3
#define REGAMOUNT 4

#define MEMACCESS(symbol) "[" + symbol + "]"
#define LABEL(label)      ".L" + to_string(label)
#define SPECIFYSIZE(r)    m_sizeSpecifiers[r - 1]
class GeneratorX86 : public Generator
{

  private:
    string m_dwordRegisters[4]  = {"eax", "ebx", "ecx", "edx"};
    string m_wordRegisters[4]   = {"ax", "bx", "cx", "dx"};
    string m_loByteRegisters[4] = {"al", "bl", "cl", "dl"};
    string m_hiByteRegisters[4] = {"ah", "bh", "ch", "dh"};
    
    string  m_sizeSpecifiers[4] = {"dword", "word", "byte", "byte"};
    string *m_registers[4]      = {m_dwordRegisters, m_wordRegisters,
                                    m_hiByteRegisters, m_loByteRegisters};
    
    int m_spilledRegisters = 0;

    /* These are 0 if unused, otherwise indexes (-1) to m_registers */
    int m_usedRegisters[4];
    
    string m_initDataSize[4] = {"db", "dw", "dd", "dq"};

private:
    void freeAllReg();
    int  allocReg();
    int  allocReg(int r);
    void spillReg(int r);
    void loadReg(int r);
    string getReg(int r);
    bool hasFreeReg();

    int checkRegisters();
    int spillAmount();
    int genLoadRegisters(vector <int> data);

  protected:
    int genLoad(int val, int size);
    int genAdd(int reg1, int reg2);
    int genSub(int reg1, int reg2);
    int genMul(int reg1, int reg2);
    int genIncrement(int sym, int amount, int after);
    int genDecrement(int sym, int amount, int after);
    int genLeftShift(int reg1, int amount);
    int genRightShift(int reg1, int amount);
    
    int _genIDiv(int reg1, int reg2, bool quotient);
    int genDiv(int reg1, int reg2);
    int genModulus(int reg1, int reg2);
    
    int genAnd(int reg1, int reg2);
    int genOr(int reg1, int reg2);
    int genXor(int reg1, int reg2);
    

    int genLoadVariable(int symbolidx, struct Type t);
    int genStoreValue(int reg, int memloc, struct Type t);

    int genCompare(int reg1, int reg2, bool clear=true);
    int genCompareSet(int op, int reg1, int reg2);
    int genFlagJump(int op, int label);

    int genJump(int label);
    int genLabel(int label);
    int genLabel(string label);
    int genGoto(string label);
    int genWidenRegister(int reg, int oldsize, int newsize, bool isSigned);
    int genPushArgument(int reg, int argindex);
    int genFunctionCall(int symbolidx, int parameters, vector<int> data);
    int genReturnJump(int reg, int funcIdx);
    int genLoadLocation(int symbolidx);
    int genPtrAccess(int reg, int size);
    int genDirectMemLoad(int offset, int symbol, int reg, int size);
    int genNegate(int reg);
    int genAccessStruct(int memreg, int idx, int size);
    int genBinNegate(int reg);
    int genIsZero(int reg);
    int genIsZeroSet(int reg, bool setOnZero);
    int genLogAnd(int reg1, int reg2);
    int genLogOr(int reg1, int reg2);
    void freeReg(int reg);
    int genMoveReg(int reg, int toReg=-1);
    vector<int> genSaveRegisters();
    
  public:
    GeneratorX86(string);
    void genDebugComment(string);
    int  genDataSection();
    int  genExternSection();

    int genFunctionPreamble(int funcInx);
    int genFunctionPostamble(int funcIdx);
};