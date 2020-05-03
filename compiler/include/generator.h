#pragma once

#include <core.h>
#include <ast.h>

class Generator
{

protected:
    FILE        *m_outfile;
    int         m_labelCount = 0;

protected:
    void write(string instruction, string source, string destination);
    void write(string instruction, int source, string destination);
    void write(string instruction);
    void write(string instruction, string destination);
    
    int generateIf(struct ast_node *tree);
    int generateWhile(struct ast_node *tree);
    int generateArgumentPush(struct ast_node *tree);
    int label();

    /* Arch dependant functions, get overwritten in arch/ARCH folder */
    virtual void freeAllReg() {}
    virtual int genLoad(int val, int size) {}
    virtual int genAdd(int reg1, int reg2) {}
    virtual int genSub(int reg1, int reg2) {}
    virtual int genMul(int reg1, int reg2) {}
    virtual int genDiv(int reg1, int reg2) {}
    virtual int genLoadVariable(int symbol) {}
    virtual int genStoreValue(int reg, int memloc, struct Type t) {}
    virtual int genCompareJump(int op, int reg1, int reg2, int label) {}
    virtual int genCompareSet(int op, int reg1, int reg2) {}
    virtual int genLabel(int label) {}
    virtual int genJump(int label) {}
    virtual int genWidenRegister(int reg, int oldsize, int newsize, bool isSigned) {}
    virtual int genPushArgument(int reg, int argindex) {}
    virtual int genFunctionCall(int symbolidx, int parameters) {}
    virtual int genReturnJump(int reg, int func) {}
    virtual int genLoadLocation(int symbolidx) {}
    virtual int genPtrAccess(int reg, int size) {}
    virtual int genDirectMemLoad(int offset, int symbol, int reg, int size) {}
    virtual int genNegate(int reg) {}
    virtual int genAccessStruct(int symbol, int idx) {}

/*debugging */
public:
    virtual int genExtern(string name) {}
    virtual int genFunctionPreamble(int funcInx) {}
    virtual int genFunctionPostamble(int funcIdx) {}
    virtual int genDataSection() {}

    Generator(string &outfile);
    void close();
    int generateFromAst(struct ast_node *tree, int reg, int parentOp);
    
    /* Debug */


};