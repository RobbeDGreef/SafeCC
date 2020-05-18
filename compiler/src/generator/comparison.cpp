#include <errorhandler.h>
#include <generator.h>
#include <symbols.h>

static int logicalNot(int op)
{
    switch(op)
    {
    case AST::Types::EQUAL: return AST::Types::NOTEQUAL;
    case AST::Types::NOTEQUAL: return AST::Types::EQUAL;
    case AST::Types::LESSTHAN: return AST::Types::GREATERTHANEQUAL;
    case AST::Types::GREATERTHANEQUAL: return AST::Types::LESSTHAN;
    case AST::Types::GREATERTHAN: return AST::Types::LESSTHANEQUAL;
    case AST::Types::LESSTHANEQUAL: return AST::Types::GREATERTHAN;
    }
    
    return 0;
}

static bool isLogOp(int op)
{
    switch (op)
    {
    case AST::Types::LOGAND:
    case AST::Types::LOGOR:
    case AST::Types::LOGNOT:
        return true;
    }
    
    return false;
}

static bool isCompareOp(int op)
{
    if (op <= AST::Types::GREATERTHANEQUAL && op >= AST::Types::EQUAL)
        return true;
    
    return false;
}

int Generator::generateComparison(ast_node *tree, 
                                  int endLabel, int parentOp)
{
    int condEndLabel = label();
    generateCondition(tree, condEndLabel, endLabel, parentOp);
    if (!isLogOp(tree->operation))
        genFlagJump(logicalNot(tree->operation), endLabel);
    
    genLabel(condEndLabel);
}

static ast_node *getRightCompLeaf(ast_node *tree)
{
    ast_node *tmp = tree;
    if (!tmp->right)
        return 0;
        
    while (isCompareOp(tmp->right->operation))
        tmp = tmp->right;
    
    return tmp;
}

static void morgansLawNegation(ast_node *tree)
{
    // Here we use De Morgans law of boolean negation
    // in a recursive manner, ei
    // and's become or's and vice versa. and the sub trees get negated as well
    switch(tree->operation)
    {
    case AST::Types::LOGAND:
        tree->operation = AST::Types::LOGOR;
        morgansLawNegation(tree->left);
        morgansLawNegation(tree->right);
        return;
        
    case AST::Types::LOGOR:
        tree->operation = AST::Types::LOGAND;
        morgansLawNegation(tree->left);
        morgansLawNegation(tree->right);
        return;

    case AST::Types::LOGNOT:
        // We can skip over these since we will automatically
        // 'correct' them when the next iteration in the ast tree happens
        morgansLawNegation(tree->left);
        return;
    }
    
    if (isCompareOp(tree->operation))
        tree->operation = logicalNot(tree->operation);
    

}

int Generator::generateCondition(ast_node *tree, int condEndLabel,
                                 int endLabel, int parentOp, int condOp)
{
    int leftreg = 0, rightreg = 0;
    int op;
    int curCondLabel = -1;
    
    switch(tree->operation)
    {
    case AST::Types::LOGNOT:
        // Thank you Augustus De Morgan :)
        morgansLawNegation(tree->left);
        generateCondition(tree->left, condEndLabel, endLabel, parentOp,
                          AST::Types::LOGNOT);
        
        if (isLogOp(tree->left->operation))
        {
            int op = getRightCompLeaf(tree->left)->operation;
            genFlagJump(logicalNot(op), endLabel);
            return -1;
        }
        
        genFlagJump(logicalNot(tree->left->operation), endLabel);
        return -1;
        
    case AST::Types::LOGOR:
        curCondLabel = label();
        leftreg = generateCondition(tree->left, condEndLabel, curCondLabel, parentOp);
        
        if (isLogOp(tree->left->operation))
            genJump(condEndLabel);
        else
            genFlagJump(tree->left->operation, condEndLabel);
        
        genLabel(curCondLabel);
        
        generateCondition(tree->right, condEndLabel, endLabel, parentOp);
        if (isCompareOp(tree->right->operation))
            genFlagJump(logicalNot(tree->right->operation), endLabel);
        
        return -1;
        
    case AST::Types::LOGAND:
        curCondLabel = label();
        leftreg = generateCondition(tree->left, condEndLabel, curCondLabel, parentOp);
        
        op = logicalNot(tree->left->operation);
        if (op)
            genFlagJump(op, endLabel);
        
        genLabel(curCondLabel);
        
        generateCondition(tree->right, condEndLabel, endLabel, parentOp);
        if (!isLogOp(tree->left->operation))
            genFlagJump(logicalNot(tree->right->operation), endLabel);
        
        return -1;
    }
    
    int ret = generateFromAst(tree, 0, parentOp);
    if (!isCompareOp(tree->operation))
    {
        if (condOp == AST::Types::LOGNOT)
            tree->operation = AST::Types::EQUAL;
        else
            tree->operation = AST::Types::NOTEQUAL;
        genIsZero(ret);
    }
    
    return -1;
}

int Generator::generateBinaryComparison(ast_node *tree, int parentOp)
{
    int condEndLabel = label();
    int reg = generateBinaryCondition(tree, condEndLabel, parentOp);
    
    if (!isLogOp(tree->operation))
        genFlagSet(logicalNot(tree->operation), reg);
    
    genLabel(condEndLabel);
    return reg;
}

int Generator::generateBinaryCondition(ast_node *tree, int endLabel,
                                 int parentOp, int condOp)
{
    int leftreg = 0, rightreg = 0;
    int op;
    int curCondLabel = -1;
    
    switch(tree->operation)
    {
    case AST::Types::LOGNOT:
        // Thank you Augustus De Morgan :)
        morgansLawNegation(tree->left);
        leftreg = generateBinaryCondition(tree->left, endLabel, parentOp, AST::Types::LOGNOT);
        
        if (isLogOp(tree->left->operation))
            op = getRightCompLeaf(tree->left)->operation;
        else
            op = tree->left->operation;
        
        genFlagSet(logicalNot(tree->left->operation), leftreg);
        return leftreg;
        
    case AST::Types::LOGOR:
        curCondLabel = label();
        leftreg = generateBinaryCondition(tree->left, curCondLabel, parentOp);
        
        if (isLogOp(tree->left->operation))
            genJump(endLabel);
        else
            genFlagJump(tree->left->operation, endLabel);
        
        freeReg(leftreg);
        genLabel(curCondLabel);
        
        return generateBinaryCondition(tree->right, endLabel, parentOp);
        
    case AST::Types::LOGAND:
        curCondLabel = label();
        leftreg = generateBinaryCondition(tree->left, curCondLabel, parentOp);
        
        op = logicalNot(tree->left->operation);
        if (op)
            genFlagJump(op, endLabel);
        
        genLabel(curCondLabel);
        
        freeReg(leftreg);
            
        rightreg = generateBinaryCondition(tree->right, endLabel, parentOp);
        op = logicalNot(tree->right->operation);
        if (op)
            genFlagJump(op, endLabel);
        
        return rightreg;
    }
    
    int ret = generateFromAst(tree, 0, parentOp);
    if (!isCompareOp(tree->operation))
    {
        if (condOp == AST::Types::LOGNOT)
        {
            tree->operation = AST::Types::EQUAL;
            ret = genIsZeroSet(ret, true);
        }
        else
        {
            tree->operation = AST::Types::NOTEQUAL;
            ret = genIsZeroSet(ret, false);
        }
    }
    
    return ret;
}
