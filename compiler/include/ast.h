#pragma once

#include <scanner.h>
#include <token.h>
#include <types.h>

namespace AST
{
    enum Types 
    {
        ADD = 1, SUBTRACT, MULTIPLY, DIVIDE, MODULUS,
        OR, XOR, AND, L_SHIFT, R_SHIFT,
        EQUAL, NOTEQUAL, LESSTHAN, GREATERTHAN, LESSTHANEQUAL, GREATERTHANEQUAL,
        LOGAND, LOGOR, LOGNOT,
        
        ASSIGN, 
        
        INCREMENT, DECREMENT, NOT,
    
        NEGATE,

        INTLIT,
        IDENTIFIER,
        
        LOADLOCATION, PTRACCESS, DIRECTMEMLOAD,
        INITIALIZER,
        
        STRUCTDEREFERENCE,
        // PADDING has no machine code, the generator will skip over it 
        PADDING, 
        WIDEN,
        LEFTVALIDENT,
        IF,
        GLUE, FUNCTION,
        WHILE,
        FUNCTIONCALL, FUNCTIONARGUMENT,
        RETURN,
        
        GOTO, LABEL, SWITCH, CASE, DEFAULT,
        
        DEBUGPRINT
    };
};

struct ast_node
{
    int              operation;
    struct ast_node *left;
    struct ast_node *mid;
    struct ast_node *right;
    int              value;
    struct Type      type;  /* size from types.h */

    /* These are to display line and char numbers of generator errors etc */
    int              line;
    int              c;
};

int tokenToAst(int token, Scanner &scanner);
struct ast_node *mkAstUnary(int operation, struct ast_node *left, int value, struct Type type, int l, int c);
struct ast_node *mkAstLeaf(int operation, int value, struct Type type, int l, int c);
struct ast_node *mkAstNode(int operation, struct ast_node *left, 
                            struct ast_node *mid, struct ast_node *right,
                            int value, struct Type type, int line, int c);

/* Some good 'ol overloaded functions (no type) */
struct ast_node *mkAstUnary(int operation, struct ast_node *left, int value,
                            int line, int c);
struct ast_node *mkAstLeaf(int operation, int value, int line, int c);
struct ast_node *mkAstNode(int operation, struct ast_node *left, 
                            struct ast_node *mid, struct ast_node *right,
                            int value, int line, int c);

struct ast_node *getRightLeaf(struct ast_node *tree);