#include <parser/parser.h>
#include <symbols.h>
#include <token.h>
#include <errorhandler.h>

struct ast_node *StatementParser::comparison()
{
    struct ast_node *cond = m_parser.m_exprParser.parseBinaryOperation(0, NULLTYPE);

#if 0
    if (cond->operation < AST::Types::EQUAL ||
        cond->operation > AST::Types::GREATERTHANEQUAL)
    {
        cond = mkAstUnary(AST::Types::ISZERO, cond, 0, cond->line, cond->c);
    }
#endif
    
    return cond;
}

struct ast_node *StatementParser::ifStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);
    struct ast_node *cond = comparison();
    m_parser.match(Token::Tokens::R_PAREN);

    struct ast_node *true_branch = parseBlock();
    struct ast_node *false_branch = 0;

    if (m_scanner.token().token() == Token::Tokens::ELSE)
    {
        m_scanner.scan();
        false_branch = parseBlock();
    }

    return mkAstNode(AST::Types::IF, cond, true_branch, false_branch, 0, cond->line, cond->c);
}

struct ast_node *StatementParser::whileStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);
    struct ast_node *cond = comparison();
    m_parser.match(Token::Tokens::R_PAREN);

    struct ast_node *loopbody = parseBlock();

    return mkAstNode(AST::Types::WHILE, cond, NULL, loopbody, 0, cond->line, cond->c);
}

struct ast_node *StatementParser::forStatement()
{
    m_scanner.scan();
    m_parser.match(Token::Tokens::L_PAREN);

    struct ast_node *forInit = parseStatement();
    m_parser.match(Token::Tokens::SEMICOLON);
    
    struct ast_node *forCond = comparison();
    m_parser.match(Token::Tokens::SEMICOLON);
    
    struct ast_node *forIter = parseStatement();
    m_parser.match(Token::Tokens::R_PAREN);
    
    struct ast_node *body = parseBlock();

    struct ast_node *tree = mkAstNode(AST::Types::GLUE, body, NULL, forIter, 0, forIter->line, forIter->c);
    tree = mkAstNode(AST::Types::WHILE, forCond, NULL, tree, 0, forCond->line, forCond->c);
    return mkAstNode(AST::Types::GLUE, forInit, NULL, tree, 0, forInit->line, forInit->c);
}

struct ast_node *StatementParser::gotoStatement()
{
    m_scanner.scan();
    m_parser.matchNoScan(Token::Tokens::IDENTIFIER);
    string labelstr = m_scanner.identifier();
    m_scanner.scan();
    
    int id;
    if ((id = g_symtable.findSymbol(labelstr)) == -1)
    {
        id = g_symtable.addSymbol(labelstr, 0, SymbolTable::SymTypes::LABEL, 0);
    }
    g_symtable.getSymbol(id)->used = true;
    
    struct ast_node *go = mkAstLeaf(AST::Types::GOTO, id, m_scanner.curLine(), 
                                       m_scanner.curChar());
    return go;
}

bool isLabelStatement(int op)
{
    if (op == AST::Types::INITIALIZER || op == AST::Types::PADDING)
        return false;
    
    return true;
}

bool isFlowStatement(int op)
{
    if (op == AST::Types::IF || op == AST::Types::WHILE)
        return true;
    
    return false;
}

struct ast_node *StatementParser::parseLabel(string label)
{
    m_scanner.scan();
    struct ast_node *left = parseStatement();
    if (left && !isFlowStatement(left->operation))
        m_parser.match(Token::Tokens::SEMICOLON);
    
    else if (left && !isLabelStatement(left->operation))
        err.fatal("Label must be placed in front of valid statement\n");
    
    int id = g_symtable.addSymbol(label, 0, SymbolTable::SymTypes::LABEL, 0);
    struct Symbol *s = g_symtable.getSymbol(id);
    s->defined = true;
    
    struct ast_node *ret = mkAstUnary(AST::Types::LABEL, left, id, left->line, left->c);
    return ret;
}
