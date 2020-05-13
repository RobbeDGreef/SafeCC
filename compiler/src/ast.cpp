#include <ast.h>
#include <core.h>
#include <errorhandler.h>

/// @brief  Creates a abstract syntax tree nodes
struct ast_node *mkAstNode(int operation, struct ast_node *left, 
                            struct ast_node *mid, struct ast_node *right,
                            int value, struct Type type, int line, int c)
{
    struct ast_node *node = new (struct ast_node);
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

struct ast_node *mkAstNode(int operation, struct ast_node *left, 
                            struct ast_node *mid, struct ast_node *right,
                            int value, int line, int c)
{
    struct Type t;
    t.primType = 0;
    return mkAstNode(operation, left, mid, right, value, t, line, c);
}

/// @brief  Creates an endpoint for the AST
struct ast_node *mkAstLeaf(int operation, int value, struct Type type, int line, int c)
{
    return mkAstNode(operation, NULL, NULL, NULL, value, type, line, c);
}

struct ast_node *mkAstLeaf(int operation, int value, int line, int c)
{
    struct Type t;
    t.primType = 0;
    return mkAstNode(operation, NULL, NULL, NULL, value, t, line, c);
}

/// @brief  Creates a unary branch of the AST
struct ast_node *mkAstUnary(int operation, struct ast_node *left, int value,
                            struct Type type, int line, int c)
{
    return mkAstNode(operation, left, NULL, NULL, value, type, line, c);
}

struct ast_node *mkAstUnary(int operation, struct ast_node *left, int value,
                            int line, int c)
{
    struct Type t;
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

struct ast_node *getRightLeaf(struct ast_node *tree)
{
    while (tree->right != NULL)
        tree = tree->right;
    
    return tree;
}