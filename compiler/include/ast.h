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
        COLONSEP, TERNARY,
        
        INCREMENT, DECREMENT, NOT,
    
        NEGATE,

        INTLIT,
        IDENTIFIER,
        
        LOADLOCATION, PTRACCESS, DIRECTMEMLOAD,
        INITIALIZER,
        
        // PADDING has no machine code, the generator will skip over it 
        PADDING, 
        WIDEN,
        LEFTVALIDENT,
        IF,
        GLUE, FUNCTION,
        WHILE, DOWHILE,
        FUNCTIONCALL, FUNCTIONARGUMENT,
        RETURN,
        
        GOTO, LABEL, SWITCH, CASE, DEFAULT,
        BREAK, CONTINUE,
        
        PUSHSCOPE, POPSCOPE,
        
        DEBUGPRINT
    };
};

struct ast_node
{
    int              operation;
    ast_node *left;
    ast_node *mid;
    ast_node *right;
    int              value;
    Type      type;  /* size from types.h */

    /* These are to display line and char numbers of generator errors etc */
    int              line;
    int              c;
};

int tokenToAst(int token, Scanner &scanner);
ast_node *mkAstUnary(int operation, ast_node *left, int value, Type type, int l, int c);
ast_node *mkAstLeaf(int operation, int value, Type type, int l, int c);
ast_node *mkAstNode(int operation, ast_node *left, 
                            ast_node *mid, ast_node *right,
                            int value, Type type, int line, int c);

/* Some good 'ol overloaded functions (no type) */
ast_node *mkAstUnary(int operation, ast_node *left, int value,
                            int line, int c);
ast_node *mkAstLeaf(int operation, int value, int line, int c);
ast_node *mkAstNode(int operation, ast_node *left, 
                            ast_node *mid, ast_node *right,
                            int value, int line, int c);

ast_node *getRightLeaf(ast_node *tree);
