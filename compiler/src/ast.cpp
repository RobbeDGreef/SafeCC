#include <ast.h>
#include <core.h>
#include <errorhandler.h>

/// @brief  Creates a abstract syntax tree nodes
ast_node *mkAstNode(int operation, ast_node *left, 
                            ast_node *mid, ast_node *right,
                            int value, Type type, int line, int c)
{
    ast_node *node = new (ast_node);
    /// @todo maybe check for allocation failures

    node->operation = operation;
    node->left      = left;
    node->mid       = mid;
    node->right     = right;
    node->value     = value;
    node->type      = type;

    node->line      = line;
    node->c         = c;

    return node;
}

ast_node *mkAstNode(int operation, ast_node *left, 
                            ast_node *mid, ast_node *right,
                            int value, int line, int c)
{
    Type t;
    t.primType = 0;
    return mkAstNode(operation, left, mid, right, value, t, line, c);
}

/// @brief  Creates an endpoint for the AST
ast_node *mkAstLeaf(int operation, int value, Type type, int line, int c)
{
    return mkAstNode(operation, NULL, NULL, NULL, value, type, line, c);
}

ast_node *mkAstLeaf(int operation, int value, int line, int c)
{
    Type t;
    t.primType = 0;
    return mkAstNode(operation, NULL, NULL, NULL, value, t, line, c);
}

/// @brief  Creates a unary branch of the AST
ast_node *mkAstUnary(int operation, ast_node *left, int value,
                            Type type, int line, int c)
{
    return mkAstNode(operation, left, NULL, NULL, value, type, line, c);
}

ast_node *mkAstUnary(int operation, ast_node *left, int value,
                            int line, int c)
{
    Type t;
    t.primType = 0;
    return mkAstNode(operation, left, NULL, NULL, value, t, line, c);
}

/// @brief  Converts tokens to AST specifiers
int tokenToAst(int token, Scanner &scanner)
{
    if (token > Token::Tokens::T_EOF && token < Token::Tokens::INTLIT)
        return token;

    err.unexpectedToken(token);
    exit(1);
}

ast_node *getRightLeaf(ast_node *tree)
{
    while (tree->right != NULL)
        tree = tree->right;
    
    return tree;
}