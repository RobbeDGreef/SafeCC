#pragma once

#include <core.h>
#include <ast.h>

class Generator
{

protected:
    FILE        *m_outfile;
    int         m_labelCount = 0;
    Scanner     *m_scanner;         // Used only for debugging

protected:
    void write(string instruction, string source, string destination);
    void write(string instruction, int source, string destination);
    void write(string instruction);
    void write(string instruction, string destination);
    
    int generateIf(struct ast_node *tree, int condLabel, int endLabel);
    int generateWhile(struct ast_node *tree);
    int generateDoWhile(struct ast_node *tree);
    int generateSwitch(struct ast_node *tree, int condLabel);
    int generateArgumentPush(struct ast_node *tree);
    int generateAssignment(struct ast_node *tree);
    int label();
    int generateCondition(struct ast_node *tree, int condEndLabel, int endLabel, int parentOp,
                          int condOp=0);
    int generateComparison(struct ast_node *tree, 
                                  int endLabel, int parentOp);
    int generateBinaryComparison(struct ast_node *tree, int parentOp);
    int generateBinaryCondition(struct ast_node *tree, int condEndLabel, int parentOp,
                          int condOp=0);
    
    int generateGoto(struct ast_node *tree);

    /* Arch dependant functions, get overwritten in arch/ARCH folder */
    virtual void freeAllReg() {}
    virtual int genLoad(int val, int size) {}
    virtual int genAdd(int reg1, int reg2) {}
    virtual int genSub(int reg1, int reg2) {}
    virtual int genMul(int reg1, int reg2) {}
    virtual int genDiv(int reg1, int reg2) {}
    virtual int genLoadVariable(int symbol, struct Type t) {}
    virtual int genStoreValue(int reg, int memloc, struct Type t) {}
    
    virtual int genCompare(int reg1, int reg2, bool clear=true) {}
    virtual int genCompareSet(int op, int reg1, int reg2) {}
    virtual int genFlagJump(int op, int label) {}
    virtual int genFlagSet(int op, int reg) {}
    
    virtual int genLabel(int label) {}
    virtual int genLabel(string label) {}
    virtual int genGoto(string label) {}
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
    virtual int genIncrement(int symbol, int amount, int after) {}
    virtual int genDecrement(int symbol, int amount, int after) {}
    virtual int genLeftShift(int reg1, int reg2) {}
    virtual int genRightShift(int reg1, int reg2) {}
    virtual int genModulus(int leftreg, int rightreg) {}
    virtual int genAnd(int reg1, int reg2) {}
    virtual int genOr(int reg1, int reg2) {}
    virtual int genXor(int reg1, int reg2) {}
    virtual int genBinNegate(int reg) {}
    virtual int genLogAnd(int reg1, int reg2) {}
    virtual int genLogOr(int reg1, int reg2) {}
    
    virtual int genIsZero(int reg) {}
    virtual int genIsZeroSet(int reg, bool setOnZero) {}
    
    virtual void freeReg(int reg) {}

/*debugging */
public:
    virtual void genDebugComment(string s) {}
    virtual int genExtern(string name) {}
    virtual int genFunctionPreamble(int funcInx) {}
    virtual int genFunctionPostamble(int funcIdx) {}
    virtual int genDataSection() {}

    Generator(string &outfile);
    void close();
    int generateFromAst(struct ast_node *tree, int reg, int parentOp, 
                        int condLabel=-1, int endLabel=-1);
    void setupInfileHandler(Scanner &scanner);
};