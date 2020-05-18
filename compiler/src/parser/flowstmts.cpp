#include <parser/parser.h>
#include <symbols.h>
#include <token.h>
#include <errorhandler.h>

ast_node *StatementParser::comparison()
{
    ast_node *cond = m_parser.m_exprParser.parseBinaryOperation(0, NULLTYPE);

#if 0
    if (cond->operation < AST::Types::EQUAL ||
        cond->operation > AST::Types::GREATERTHANEQUAL)
    {
        cond = mkAstUnary(AST::Types::ISZERO, cond, 0, cond->line, cond->c);
    }
#endif
    
    return cond;
}

ast_node *StatementParser::ifStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);
    ast_node *cond = comparison();
    m_parser.match(Token::Tokens::R_PAREN);

    ast_node *true_branch = parseBlock();
    ast_node *false_branch = NULL;

    if (m_scanner.token().token() == Token::Tokens::ELSE)
    {
        m_scanner.scan();
        false_branch = parseBlock();
    }

    return mkAstNode(AST::Types::IF, cond, true_branch, false_branch, 0, cond->line, cond->c);
}

ast_node *StatementParser::whileStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);
    ast_node *cond = comparison();
    m_parser.match(Token::Tokens::R_PAREN);

    ast_node *loopbody = parseBlock(Token::Tokens::WHILE);

    return mkAstNode(AST::Types::WHILE, cond, NULL, loopbody, 0, cond->line, cond->c);
}

static ast_node *checkPadding(ast_node *tree)
{
    if (tree->operation == AST::Types::PADDING)
    {
        delete tree;
        return NULL;
    }
    
    return tree;
}

ast_node *StatementParser::forStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);
    
    int l = m_scanner.curLine();
    int c = m_scanner.curChar();

    int id = g_symtable.newScope();
    ast_node *forInit = parseStatement();
    forInit = checkPadding(forInit);
    m_parser.match(Token::Tokens::SEMICOLON);
    
    ast_node *push = mkAstLeaf(AST::Types::PUSHSCOPE, id, 0, 0);
    forInit = mkAstNode(AST::Types::GLUE, push, NULL, forInit, 0, 0, 0);
    
    ast_node *forCond = comparison();
    forCond = checkPadding(forCond);
    m_parser.match(Token::Tokens::SEMICOLON);
    
    ast_node *forIter = parseStatement();
    forIter = checkPadding(forIter);
    m_parser.match(Token::Tokens::R_PAREN);
    
    ast_node *body = parseBlock(Token::Tokens::FOR, false);
    ast_node *tree = mkAstNode(AST::Types::GLUE, body, NULL, forIter, 0, l, c);

    // Value is 1, in the generator we interpret this flag as being a FOR loop
    tree = mkAstNode(AST::Types::WHILE, forCond, NULL, tree, 1, l, c);
    tree = mkAstNode(AST::Types::GLUE, forInit, NULL, tree, 0, l, c);
    
    ast_node *pop = mkAstLeaf(AST::Types::POPSCOPE, 0, 0, 0);
    tree = mkAstNode(AST::Types::GLUE, tree, NULL, pop, 0, 0, 0);
    g_symtable.popScope();

    return tree;
}

ast_node *StatementParser::gotoStatement()
{
    m_scanner.scan();
    m_parser.matchNoScan(Token::Tokens::IDENTIFIER);
    string labelstr = m_scanner.identifier();
    m_scanner.scan();
    
    int id;
    Symbol sym;
    if ((id = g_symtable.findSymbol(labelstr)) == -1)
    {
        sym = g_symtable.createSymbol(labelstr, 0, SymbolTable::SymTypes::LABEL,
                                      NULLTYPE, 0);
        id = g_symtable.addToFunction(sym);
    }
    g_symtable.getSymbol(id)->used = true;
    
    ast_node *go = mkAstLeaf(AST::Types::GOTO, id, m_scanner.curLine(), 
                                       m_scanner.curChar());
    return go;
}

bool isLabelStatement(int op)
{
    if (op == AST::Types::INITIALIZER || op == AST::Types::PADDING)
        return false;
    
    return true;
}

ast_node *StatementParser::parseLabel(string label)
{
    m_scanner.scan();
    
    ast_node *left = parseStatement();
    
    if (left->operation == AST::Types::PADDING)
        left = NULL;
    
    if (left && !isLabelStatement(left->operation))
        err.fatal("Label must be placed in front of valid statement\n");
    
    int id;
    Symbol sym;
    if ((id = g_symtable.findSymbol(label)) == -1)
    {
        sym = g_symtable.createSymbol(label, 0, SymbolTable::SymTypes::LABEL,
                                      NULLTYPE, 0);
        id = g_symtable.addToFunction(sym);
    }
    
    Symbol *s = g_symtable.getSymbol(id);
    s->defined = true;

    int l = m_scanner.curLine();
    int c = m_scanner.curChar();
    ast_node *ret = mkAstUnary(AST::Types::LABEL, left, id, l, c);
    return ret;
}

static bool isSwitchFlowKeyword(int op)
{
    if (op == AST::Types::CASE || op == AST::Types::DEFAULT)
        return true;
    
    return false;
}

// Warning: this piece of code is somewhat complex, treat it with care
ast_node *StatementParser::switchWalk(ast_node *tree)
{
    ast_node *code = NULL;
    ast_node *cases = NULL;
    ast_node *lastCodeIter = NULL;
    ast_node *codeHead = tree;
    ast_node *prevCode = NULL;
    
    while (tree)
    {
        if (tree->operation == AST::Types::GLUE)
            code = tree->right;
        else
            code = tree;
            
        switch(code->operation)
        {
        case AST::Types::DEFAULT:
        case AST::Types::CASE:
            if (lastCodeIter)
                lastCodeIter->left = NULL;
            
            code->right = cases;
            cases = code;
            
            cases->left = codeHead;
            break;
        
        default:
            if (!prevCode || isSwitchFlowKeyword(prevCode->operation))
                codeHead = tree;
            
            lastCodeIter = tree;
            break;
        }
        
        prevCode = code;
        tree = tree->left;
    }
    
    return cases;
}

ast_node *StatementParser::switchStatement()
{
    ast_node *expr;
    ast_node *body;
    ast_node *tree;
    
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);
    expr = m_parser.m_exprParser.parseBinaryOperation(0, NULLTYPE);
    
    int id = g_symtable.newScope();
    ast_node *push = mkAstLeaf(AST::Types::PUSHSCOPE, id, 0, 0);
    expr = mkAstNode(AST::Types::GLUE, expr, NULL, push, 0, 0, 0);
    
    m_parser.match(Token::Tokens::R_PAREN);
    body = parseBlock(Token::Tokens::SWITCH, false);
    body = switchWalk(body);
    
    
    tree = mkAstNode(AST::Types::SWITCH, expr, NULL, body, 0,
                     m_scanner.curLine(), m_scanner.curChar());
    
    g_symtable.popScope();
    ast_node *pop = mkAstLeaf(AST::Types::POPSCOPE, 0, 0, 0);
    return mkAstNode(AST::Types::GLUE, tree, NULL, pop, 0, 0, 0);
}


ast_node *StatementParser::switchCaseStatement()
{
    m_scanner.scan();
    int constant = m_parser.m_exprParser.parseConstantExpr();
    m_parser.match(Token::Tokens::COLON);
    
    return mkAstLeaf(AST::Types::CASE, constant, m_scanner.curLine(),
                     m_scanner.curChar());
}

ast_node *StatementParser::switchDefaultStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::COLON);
    
    return mkAstLeaf(AST::Types::DEFAULT, 0, m_scanner.curLine(),
                     m_scanner.curChar());
}

ast_node *StatementParser::doWhileStatement()
{
    m_scanner.scan();
    ast_node *body = parseBlock(AST::Types::WHILE);
    m_parser.match(Token::Tokens::WHILE);
    m_parser.match(Token::Tokens::L_PAREN);
    ast_node *cond = comparison();
    m_parser.match(Token::Tokens::R_PAREN);
    m_parser.match(Token::Tokens::SEMICOLON);
    
    return mkAstNode(AST::Types::DOWHILE, cond, NULL, body, 0, cond->line, cond->c);
}