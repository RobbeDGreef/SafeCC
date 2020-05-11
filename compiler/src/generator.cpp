#include <errorhandler.h>
#include <generator.h>
#include <symbols.h>

int Generator::label()
{
    return m_labelCount++;
}

int Generator::generateIf(struct ast_node *tree)
{
    int falseLabel = -1;
    int endLabel = -1;
    falseLabel = label();
    if (tree->right)
        endLabel = label();

    generateComparison(tree->left, falseLabel, tree->operation);
    freeAllReg();
    
    generateFromAst(tree->mid, -1, tree->operation);
    freeAllReg();

    if (tree->right)
        genJump(endLabel);

    genLabel(falseLabel);

    if (tree->right)
    {
        generateFromAst(tree->right, -1, tree->operation);
        freeAllReg();
        genLabel(endLabel);
    }

    return -1;
}

int Generator::generateWhile(struct ast_node *tree)
{
    int startLabel = label();
    int endLabel   = label();

    genLabel(startLabel);
    generateComparison(tree->left, endLabel, tree->operation);
    freeAllReg();
    generateFromAst(tree->right, -1, tree->operation);
    freeAllReg();
    genJump(startLabel);
    genLabel(endLabel);

    return -1;
}

int Generator::generateArgumentPush(struct ast_node *tree)
{
    if (tree->right->operation == AST::Types::IDENTIFIER && 
        tree->right->type.typeType == TypeTypes::STRUCT &&
        !tree->right->type.ptrDepth)
    {
        vector<struct StructItem> sitems = tree->right->type.contents;
        for (int i = sitems.size() - 1; i >= 0; i--)
        {
            int right = genAccessStruct(tree->right->value, i);
            genPushArgument(right, tree->value);
        }
    }
    else
    {
        int right = generateFromAst(tree->right, -1, tree->operation);
        genPushArgument(right, tree->value);
    }
    generateFromAst(tree->left, -1, tree->operation);
    
    return -1;
}

static int countDepth(struct ast_node *tree)
{
    int i = 0;
    while ((tree = tree->left) != NULL)
        i++;

    return i;
}

int Generator::generateAssignment(struct ast_node *tree)
{
    struct ast_node *left;
    int l = tree->left->line;
    int c = tree->left->c;
    
    if (tree->left->operation == AST::Types::IDENTIFIER)
        left = mkAstLeaf(AST::Types::LOADLOCATION, tree->left->value, tree->type, l, c);
    else if (tree->left->operation == AST::Types::PTRACCESS)
        left = tree->left->left;
    else
        left = tree->left;

    int lreg = generateFromAst(left, 0, AST::Types::ASSIGN);
    int rreg = generateFromAst(tree->right, 0, AST::Types::ASSIGN);
    
    return genStoreValue(rreg, lreg, tree->type);
}

static bool isFlowStatement(int tok)
{
    if (tok == AST::Types::IF || tok == AST::Types::WHILE)
        return true;
    
    return false;
}

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

static bool isLogOp(int tok)
{
    switch (tok)
    {
    case AST::Types::LOGAND:
    case AST::Types::LOGOR:
    case AST::Types::LOGNOT:
        return true;
    }
    
    return false;
}

static bool isCompareOp(int tok)
{
    if (tok <= AST::Types::GREATERTHANEQUAL && tok >= AST::Types::EQUAL)
        return true;
    
    return false;
}

int Generator::generateComparison(struct ast_node *tree, 
                                  int endLabel, int parentOp)
{
    int condEndLabel = label();
    generateCondition(tree, condEndLabel, endLabel, parentOp);
    if (!isLogOp(tree->operation))
        genFlagJump(logicalNot(tree->operation), endLabel);
    
    genLabel(condEndLabel);
}

static ast_node *getRightLeaf(struct ast_node *tree)
{
    struct ast_node *tmp = tree;
    if (!tmp->right)
        return 0;
        
    while (isCompareOp(tmp->right->operation))
        tmp = tmp->right;
    
    return tmp;
}

static void morgansLawNegation(struct ast_node *tree)
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

int Generator::generateCondition(struct ast_node *tree, int condEndLabel,
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
            int op = getRightLeaf(tree->left)->operation;
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
            genFlagJump(tree->left->operation, endLabel);
        
        genLabel(curCondLabel);
        
        generateCondition(tree->right, condEndLabel, endLabel, parentOp);
        if (isCompareOp(tree->right->operation))
            genFlagJump(logicalNot(tree->right->operation), endLabel);
        
        return -1;
        
    case AST::Types::LOGAND:
        leftreg = generateCondition(tree->left, condEndLabel, endLabel, parentOp);
        
        op = logicalNot(tree->left->operation);
        if (op)
            genFlagJump(op, endLabel);
            
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

int Generator::generateBinaryComparison(struct ast_node *tree, int parentOp)
{
    int condEndLabel = label();
    int reg = generateBinaryCondition(tree, condEndLabel, parentOp);
    
    if (!isLogOp(tree->operation))
        genFlagSet(logicalNot(tree->operation), reg);
    
    genLabel(condEndLabel);
    
    return reg;
}

int Generator::generateBinaryCondition(struct ast_node *tree, int endLabel,
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
            op = getRightLeaf(tree->left)->operation;
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
        leftreg = generateBinaryCondition(tree->left, endLabel, parentOp);
        
        op = logicalNot(tree->left->operation);
        if (op)
            genFlagJump(op, endLabel);
        
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
            tree->operation = AST::Types::EQUAL;
        else
            tree->operation = AST::Types::NOTEQUAL;
        genIsZeroSet(ret);
    }
    
    return ret;
}



int Generator::generateFromAst(struct ast_node *tree, int reg, int parentOp)
{
    int leftreg = 0;
    int rightreg = 0;

    /* If no instructions are given just return */
    if (!tree)
        return -1;
    
    DEBUG("op: " << tree->operation)

    switch (tree->operation)
    {
    case AST::Types::GLUE:
        generateFromAst(tree->left, -1, tree->operation);
        freeAllReg();
        generateFromAst(tree->right, -1, tree->operation);
        freeAllReg();
        return -1;
        
    case AST::Types::IF:
        return generateIf(tree);
    case AST::Types::WHILE:
        return generateWhile(tree);
    case AST::Types::FUNCTION:
        genFunctionPreamble(tree->value);
        generateFromAst(tree->left, -1, tree->operation);
        genFunctionPostamble(tree->value);
        return -1;
        
    case AST::Types::FUNCTIONARGUMENT:
        // Because arguments are pushed in reverse order
        generateArgumentPush(tree);
        return -1;
        
    case AST::Types::INCREMENT:
        DEBUG("tree l " << tree->left << " r " << tree->right)
        return genIncrement(tree->left->value, tree->right->value, tree->value);
    case AST::Types::DECREMENT:
        leftreg = generateFromAst(tree->left, -1, tree->operation);
        return genDecrement(leftreg, tree->right->value, tree->value);
    case AST::Types::ASSIGN:
        return generateAssignment(tree);
        
    case AST::Types::LOGAND:
    case AST::Types::LOGNOT:
    case AST::Types::LOGOR:
        return generateBinaryComparison(tree, tree->operation);

    case AST::Types::DEBUGPRINT:
        string comment = m_scanner->getStrFromTo(tree->value, tree->c);
        genDebugComment(comment);
        return generateFromAst(tree->left, -1, 0);
    
    }

    if (tree->left)
        leftreg = generateFromAst(tree->left, -1, tree->operation);

    if (tree->right)
        rightreg = generateFromAst(tree->right, leftreg, tree->operation);

    switch (tree->operation)
    {
    case AST::Types::ADD:
        return genAdd(leftreg, rightreg);
    case AST::Types::SUBTRACT:
        return genSub(leftreg, rightreg);
    case AST::Types::MULTIPLY:
        return genMul(leftreg, rightreg);
    case AST::Types::DIVIDE:
        return genDiv(leftreg, rightreg);
    case AST::Types::L_SHIFT:
        return genLeftShift(leftreg, rightreg);
    case AST::Types::R_SHIFT:
        return genRightShift(leftreg, rightreg);
    case AST::Types::MODULUS:
        return genModulus(leftreg, rightreg);
    case AST::Types::AND:
        return genAnd(leftreg, rightreg);
    case AST::Types::OR:
        return genOr(leftreg, rightreg);
    case AST::Types::XOR:
        return genXor(leftreg, rightreg);
    
    case AST::Types::NOT:
        return genBinNegate(leftreg);
    
    case AST::Types::INTLIT:
        return genLoad(tree->value, tree->type.size);
    case AST::Types::IDENTIFIER:
        return genLoadVariable(tree->value, tree->type);
    case AST::Types::WIDEN:
        return genWidenRegister(leftreg, tree->value, tree->type.size,
                                tree->type.isSigned);

    case AST::Types::PTRACCESS:
        return genPtrAccess(leftreg, tree->type.size);
        
    case AST::Types::EQUAL:
    case AST::Types::NOTEQUAL:
    case AST::Types::LESSTHAN:
    case AST::Types::LESSTHANEQUAL:
    case AST::Types::GREATERTHAN:
    case AST::Types::GREATERTHANEQUAL:
        if (!isFlowStatement(parentOp))
            return genCompareSet(tree->operation, leftreg, rightreg);
        
        return genCompare(tree->operation, leftreg, rightreg);

    case AST::Types::FUNCTIONCALL:
        DEBUG("GENERATING FUNC CALL")
        return genFunctionCall(tree->value, countDepth(tree));
    case AST::Types::RETURN:
        return genReturnJump(leftreg, tree->value);

    case AST::Types::LOADLOCATION:
        if (!tree->left)
            return genLoadLocation(tree->value);
        
        DEBUG("leftreg: " << leftreg)
        return leftreg;
    
    case AST::Types::DIRECTMEMLOAD:
        return genDirectMemLoad(tree->value, tree->mid->value, rightreg,
                                tree->type.size);

    case AST::Types::NEGATE:
        return genNegate(leftreg);
    
    case AST::Types::INITIALIZER:
        return leftreg;

    default:
        // This is more of a debugging check then a release thing because
        // we should normally never get here unless something is unimplemented
        // or seriously wrong (ei memory bug)
        err.fatal("Unknown AST operator " + to_string(tree->operation));
    }
}

Generator::Generator(string &outfile)
{
    m_outfile = fopen(outfile.c_str(), "w");

    if (m_outfile == NULL)
        err.fatal("Could not open file: '" + outfile + "'");
}

void Generator::close()
{
    int ex = fclose(m_outfile);
    if (ex == EOF)
        err.fatal("Could not close file");
}

void Generator::write(string instruction, string source, string destination)
{
    fprintf(m_outfile, "\t%s\t%s, %s\n", instruction.c_str(),
            destination.c_str(), source.c_str());
}

void Generator::write(string instruction, string destination)
{
    fprintf(m_outfile, "\t%s\t%s\n", instruction.c_str(), destination.c_str());
}

void Generator::write(string instruction)
{
    fprintf(m_outfile, "\t%s\n", instruction.c_str());
}

void Generator::write(string instruction, int source, string destination)
{
    write(instruction, to_string(source), destination);
}

void Generator::setupInfileHandler(Scanner &scanner)
{
    m_scanner = &scanner;
}