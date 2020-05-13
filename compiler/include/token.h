#pragma once

#include <core.h>
#include <string>

static const string toknames[]
{
    "EOF (end of file)", 
    "+ (plus)", "- (minus)", "* (star)", "/ (slash)", "% (percent sign)",
    "| (bitwise or)", "^ (bitwise xor)", "& (ampersant)", "<< (left shift)", ">> (right shift)",
    
    "== (equal to)", "!= (not equal to)", "< (less than)", "> (greater than)", 
    "<= (less than or equal to)", ">= (greater than or equal to)",
    "&& (logical and)", "|| (logical or)",
    "! (logical not)", 

    "++ (increment)", "-- (decrement)", "~ (bitwise negate)",

    "integer literal", "string literal",
    "identifier",
    "= (equalsign)", "; (semicolon)", "{ (left brace)", "} (right brace)", 
    "( (left parenthesis)", ") (right parenthesis)", ", (comma)",
    "[ (left bracket)", "] (right bracket)", ". (dot)",
    
    "void", "char", "short", "int", "long", "unsigned", "signed", "const",
    "if", "else", "while", "for", "return", 
    "sizeof",
    "typedef", "struct", "union", "enum",
    "auto", "static", "register", "extern",
    "restrict", "attribute", "asm",
    "goto", "colon", "switch", "case", "default"
};

class Token
{
private:
    int     m_token = 0;
    int     m_intValue = 0;
    
    int     m_startLine = 0;
    int     m_endLine = 0;
    int     m_startCol = 0;
    int     m_endCol = 0;

public:
    enum Tokens
    {
        T_EOF = 0, /* Prefixed to avoid builtin constant EOF */
        
        /* Operators */
        PLUS, MINUS, STAR, SLASH, MODULUS,
        OR, XOR, AMPERSANT, L_SHIFT, R_SHIFT, 
        
        /* Comparisons */
        EQUAL, NOTEQUAL, LESSTHAN, GREATERTHAN, LESSTHANEQUAL, GREATERTHANEQUAL,
        
        /* Logical operators */
        LOGAND, LOGOR, 
        LOGNOT,
        
        /* assignment */
        EQUALSIGN,
        
        INC, DEC, TIDDLE,

        INTLIT, STRINGLIT,
        IDENTIFIER,
        
        /* Punctuation */
        SEMICOLON, L_BRACE, R_BRACE, L_PAREN, R_PAREN, COMMA,
        L_BRACKET, R_BRACKET, DOT,

        /* Keywords */
        VOID, CHAR, SHORT, INT, LONG, UNSIGNED, SIGNED, CONST,
        IF, ELSE, WHILE, FOR,
        RETURN, 
        SIZEOF,
        TYPEDEF, STRUCT, UNION, ENUM,
        AUTO, STATIC, REGISTER, EXTERN,
        RESTRICT, ATTRIBUTE, ASM,
        GOTO, COLON, SWITCH, CASE, DEFAULT
    };

    int token();
    int intValue();
    Token *previousToken();
    void set(int tok, int value, int line, int col);
    void set(int tok, int line, int col);
    
    int startLine();
    int startCol();
    int endLine();
    int endCol();
};


string tokToStr(int token);
int isArithmetic(int tok);