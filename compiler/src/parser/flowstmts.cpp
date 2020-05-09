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