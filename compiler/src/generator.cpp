#include <errorhandler.h>
#include <generator.h>
#include <symbols.h>

int Generator::label()
{
    return m_labelCount++;
}

int Generator::generateIf(struct ast_node *tree)
{
    int falseLabel, endLabel = -1;

    falseLabel = label();
    if (tree->right)
        endLabel = label();

    generateFromAst(tree->left, falseLabel, tree->operation);
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
    generateFromAst(tree->left, endLabel, tree->operation);
    freeAllReg();
    generateFromAst(tree->right, -1, tree->operation);
    freeAllReg();
    genJump(startLabel);
    genLabel(endLabel);

    return -1;
}

int Generator::generateArgumentPush(struct ast_node *tree)
{
    #if 0
    if (tree->right->operation == AST::Types::IDENTIFIER && 
        tree->right->type.typeType == TypeTypes::STRUCT &&
        !tree->right->type.ptrDepth)
    {
        vector<struct StructItem> sitems = tree->right->type.structContents;
        for (int i = sitems.size() - 1; i >= 0; i--)
        {
            int right = genAccessStruct(tree->right->value, i);
            genPushArgument(right, tree->value);
        }
    }
    else
    {
    #endif
        int right = generateFromAst(tree->right, -1, tree->operation);
        genPushArgument(right, tree->value);
    //}
    generateFromAst(tree->left, -1, tree->operation);
    
    return -1;
}

int countDepth(struct ast_node *tree)
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
    case AST::Types::IF:
        return generateIf(tree);
    case AST::Types::GLUE:
        generateFromAst(tree->left, -1, tree->operation);
        freeAllReg();
        generateFromAst(tree->right, -1, tree->operation);
        freeAllReg();
        return -1;
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
    case AST::Types::INTLIT:
        return genLoad(tree->value, tree->type.size);
    case AST::Types::IDENTIFIER:
        return genLoadVariable(tree->value, tree->type);
    case AST::Types::WIDEN:
        /**
         *  @todo   i could probably integrate this widen token into
         * genLoadGlobal() and reducing asm instructions
         *
         * also this is a really hacky fix
         */
        return genWidenRegister(leftreg, tree->value, tree->type.size,
                                tree->type.isSigned);

    //case AST::Types::LEFTVALIDENT:
    //    return genStoreValue(reg, leftreg, tree->type);
    case AST::Types::PTRACCESS:
        return genPtrAccess(leftreg, tree->type.size);
    case AST::Types::EQUAL:
    case AST::Types::NOTEQUAL:
    case AST::Types::LESSTHAN:
    case AST::Types::LESSTHANEQUAL:
    case AST::Types::GREATERTHAN:
    case AST::Types::GREATERTHANEQUAL:
        if (parentOp == AST::Types::IF || parentOp == AST::Types::WHILE)
            return genCompareJump(tree->operation, leftreg, rightreg, reg);
        else
            return genCompareSet(tree->operation, leftreg, rightreg);

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