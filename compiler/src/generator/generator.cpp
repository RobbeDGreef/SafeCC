#include <errorhandler.h>
#include <generator.h>
#include <symbols.h>

int Generator::label()
{
    return m_labelCount++;
}

int Generator::generateIf(struct ast_node *tree, int condLabel, int parentEndLabel)
{
    int falseLabel = -1;
    int endLabel = -1;
    falseLabel = label();
    if (tree->right)
        endLabel = label();

    generateComparison(tree->left, falseLabel, tree->operation);
    freeAllReg();
    
    generateFromAst(tree->mid, -1, tree->operation, condLabel, parentEndLabel);
    freeAllReg();

    if (tree->right)
        genJump(endLabel);

    genLabel(falseLabel);

    if (tree->right)
    {
        generateFromAst(tree->right, -1, tree->operation, condLabel, parentEndLabel);
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
    if (tree->left)
        generateComparison(tree->left, endLabel, tree->operation);
    freeAllReg();
    
    // tree->value will equal 1 when this is actually a for loop
    if (tree->value)
    {
        int contLabel = label();
        generateFromAst(tree->right->left, -1, tree->operation, contLabel, endLabel);
        genLabel(contLabel);
        generateFromAst(tree->right->right, -1, tree->operation, contLabel, endLabel);
    }
    else
        generateFromAst(tree->right, -1, tree->operation, startLabel, endLabel);
        
    freeAllReg();
    genJump(startLabel);
    genLabel(endLabel);

    return -1;
}

int Generator::generateDoWhile(struct ast_node *tree)
{
    int startLabel = label();
    int condLabel = label();
    int endLabel = label();
    
    genLabel(startLabel);
    generateFromAst(tree->right, endLabel, tree->operation, condLabel, endLabel);
    freeAllReg();
    
    genLabel(condLabel);
    generateComparison(tree->left, endLabel, tree->operation);
    genJump(startLabel);
    
    freeAllReg();
    genLabel(endLabel);
    
    return -1;
}

int Generator::generateArgumentPush(struct ast_node *tree)
{
    if (tree->right->type.typeType == TypeTypes::STRUCT && !tree->right->type.ptrDepth)
    {
        int reg = generateFromAst(tree->right, -1, tree->operation);
        
        vector<struct StructItem> sitems = tree->right->type.contents;
        for (int i = sitems.size() - 1; i >= 0; i--)
        {
            int right = genAccessStruct(reg, sitems[i].offset, sitems[i].itemType.size);
            genPushArgument(right, tree->value);
        }
        
        freeReg(reg);
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

int Generator::generateSwitch(struct ast_node *tree, int condLabel)
{
    struct ast_node *caseIter = tree->right;
    
    // Generate the pushscope instruction
    generateFromAst(tree->left->right, -1, 0);
    
    if (tree->left->operation == AST::Types::GLUE)
        tree->left = tree->left->left;
        
    int exprReg = generateFromAst(tree->left, -1, tree->operation);
    
    vector<int> caseLabels;
    int caseLabel;
    int endLabel = label();
    bool defaultFlag = false;
    
    // Generating branchtable   
    while (caseIter)
    {
        if (caseIter->operation == AST::Types::DEFAULT)
        {
            caseLabels.push_back(endLabel);
            caseIter = caseIter->right;
            defaultFlag = true;
            continue;
        }
        
        int compReg = genLoad(caseIter->value, tree->left->type.size);
        caseLabel = label();
        caseLabels.push_back(caseLabel);
        
        genCompare(exprReg, compReg, false);
        genFlagJump(AST::Types::EQUAL, caseLabel);
        caseIter = caseIter->right;
    }
    
    genJump(endLabel);
    if (defaultFlag)
        endLabel = label();
    
    // Reloop the caselist to generate the statements
    caseIter = tree->right;
    int i = 0;
    while (caseIter)
    {    
        genLabel(caseLabels[i]);
        
        // This freeReg() call is just a safety mechanism
        int reg = generateFromAst(caseIter->left, -1, tree->operation, condLabel, endLabel);
        if (reg != -1)
            freeReg(reg);
        
        i++;
        caseIter = caseIter->right;
    }
    
    genLabel(endLabel);
    return -1;
}

int Generator::generateGoto(struct ast_node *tree)
{
    struct Symbol *s = g_symtable.getSymbol(tree->value);
    
    if (!s->defined)
        err.fatal("Label " + HL(s->name) + " undefined", tree->line, tree->c);
    
    genGoto(s->name);
}

int Generator::generateTernary(struct ast_node *tree)
{
    int falseLabel = label();
    int endLabel = label();
    int reg = -1;
    int out = -1;
    
    generateComparison(tree->left, falseLabel, AST::Types::IF);
    
    reg = generateFromAst(tree->right->left, -1, AST::Types::IF, -1, -1);
    if (reg != -1)
        out = genMoveReg(reg, -1);

    genJump(endLabel);
    genLabel(falseLabel);

    reg = generateFromAst(tree->right->right, -1, AST::Types::IF, -1, -1);
    if (reg != -1)
        out = genMoveReg(reg, out);
        
    genLabel(endLabel);
    return out;
}

static bool isFlowStatement(int op)
{
    if (op == AST::Types::IF || op == AST::Types::WHILE ||
        op == AST::Types::DOWHILE || op == AST::Types::SWITCH)
        return true;
    
    return false;
}

int Generator::generateFromAst(struct ast_node *tree, int reg, int parentOp, 
                               int condLabel, int endLabel)
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
        generateFromAst(tree->left, -1, tree->operation, condLabel, endLabel);
        freeAllReg();
        generateFromAst(tree->right, -1, tree->operation, condLabel, endLabel);
        freeAllReg();
        return -1;
        
    case AST::Types::IF:
        return generateIf(tree, condLabel, endLabel);
    case AST::Types::WHILE:
        return generateWhile(tree);
    case AST::Types::DOWHILE:
        return generateDoWhile(tree);
    case AST::Types::SWITCH:
        return generateSwitch(tree, condLabel);
    case AST::Types::FUNCTION:
        genFunctionPreamble(tree->value);
        generateFromAst(tree->left, -1, tree->operation, condLabel, endLabel);
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
        leftreg = generateFromAst(tree->left, -1, tree->operation, condLabel, endLabel);
        return genDecrement(leftreg, tree->right->value, tree->value);
    case AST::Types::ASSIGN:
        leftreg = generateAssignment(tree);
        if (parentOp == 0)
            freeReg(leftreg);
        
        return leftreg;
        
    case AST::Types::LOGAND:
    case AST::Types::LOGNOT:
    case AST::Types::LOGOR:
        return generateBinaryComparison(tree, tree->operation);
    
    case AST::Types::LABEL:
        genLabel(g_symtable.getSymbol(tree->value)->name);
        return generateFromAst(tree->left, 0, tree->operation, condLabel, endLabel);
    
    case AST::Types::TERNARY:
        return generateTernary(tree);

    case AST::Types::FUNCTIONCALL:
        DEBUG("GENERATING FUNC CALL")
        {
            vector <int> data;
            if (!(tree->type.typeType == TypeTypes::STRUCT && !tree->type.ptrDepth))
                data = genSaveRegisters();
            generateFromAst(tree->left, -1, tree->operation);
            return genFunctionCall(tree->value, countDepth(tree), data);    
        }
    
    case AST::Types::DEBUGPRINT:
        string comment = m_scanner->getStrFromTo(tree->value, tree->c);
        genDebugComment(comment);
        return generateFromAst(tree->left, -1, 0, condLabel, endLabel);
    
    }

    if (tree->left)
        leftreg = generateFromAst(tree->left, -1, tree->operation, condLabel, endLabel);

    if (tree->right)
        rightreg = generateFromAst(tree->right, leftreg, tree->operation, condLabel, endLabel);

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
        
        return genCompare(leftreg, rightreg);

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
        
    case AST::Types::GOTO:
        generateGoto(tree);
        return -1;
    
    case AST::Types::CONTINUE:
        if (condLabel == -1)
            err.fatal("Continue statements are only allowed inside for and while loops", tree->line, tree->c);
        
        genJump(condLabel);
        return -1;
    
    case AST::Types::BREAK:
        if (endLabel == -1)
            err.fatal("Break statements are only allowed inside switch, for and while loops", tree->line, tree->c);
        
        genJump(endLabel);
        return -1;
    
    case AST::Types::PUSHSCOPE:
        g_symtable.pushScopeById(tree->value);
        return -1;
    
    case AST::Types::POPSCOPE:
        g_symtable.popScope(false);
        return -1;
    
    case AST::Types::PADDING:
        err.warning("yea you should probably not be seeing this");
        return -1;

    default:
        // This is more of a debugging check then a release thing because
        // we should normally never get here unless something is unimplemented
        // or seriously wrong (ei memory bug)
        err.fatal("Unknown AST operator " + to_string(tree->operation), tree->line, tree->c);
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

void Generator::move(string instr, string source_reg, string dest_reg)
{
    /* Little optimization, do not move same reg in same reg */
    if (source_reg.compare(dest_reg))
        write(instr, source_reg, dest_reg);
}