#include <core.h>
#include <scanner.h>
#include <token.h>

int Scanner::identifyKeyword(string keyword)
{
    // Breath first ish optimized keyword scanner
    switch (keyword[0])
    {
    case 'a':
        if (!keyword.compare("auto"))
            return Token::Tokens::AUTO;
        break;
        
    case 'b':
        if (!keyword.compare("break"))
            return Token::Tokens::BREAK;
        break;
        
    case 'c':
        if (!keyword.compare("char"))
            return Token::Tokens::CHAR;
        else if (!keyword.compare("const"))
            return Token::Tokens::CONST;
        else if (!keyword.compare("case"))
            return Token::Tokens::CASE;
        else if (!keyword.compare("continue"))
            return Token::Tokens::CONTINUE;
        break;
    
    case 'd':
        if (!keyword.compare("default"))
            return Token::Tokens::DEFAULT;
        else if (!keyword.compare("do"))
            return Token::Tokens::DO;
        else if (!keyword.compare("double"))
            return Token::Tokens::DOUBLE;
        break;
        
    case 'e':
        if (!keyword.compare("else"))
            return Token::Tokens::ELSE;
        else if (!keyword.compare("extern"))
            return Token::Tokens::EXTERN;
        else if (!keyword.compare("enum"))
            return Token::Tokens::ENUM;
        break;

    case 'f':
        if (!keyword.compare("for"))
            return Token::Tokens::FOR;
        else if (!keyword.compare("float"))
            return Token::Tokens::FLOAT;
        break;

    case 'g':
        if (!keyword.compare("goto"))
            return Token::Tokens::GOTO;
        break;
        
    case 'i':
        if (!keyword.compare("int"))
            return Token::Tokens::INT;
        else if (!keyword.compare("if"))
            return Token::Tokens::IF;
        break;

    case 'l':
        if (!keyword.compare("long"))
            return Token::Tokens::LONG;
        break;

    case 'r':
        if (!keyword.compare("return"))
            return Token::Tokens::RETURN;
        else if (!keyword.compare("register"))
            return Token::Tokens::REGISTER;
        break;

    case 's':
        if (!keyword.compare("short"))
            return Token::Tokens::SHORT;
        else if (!keyword.compare("signed"))
            return Token::Tokens::SIGNED;
        else if (!keyword.compare("struct"))
            return Token::Tokens::STRUCT;
        else if (!keyword.compare("static"))
            return Token::Tokens::STATIC;
        else if (!keyword.compare("sizeof"))
            return Token::Tokens::SIZEOF;
        else if (!keyword.compare("switch"))
            return Token::Tokens::SWITCH;
        break;

    case 't':
        if (!keyword.compare("typedef"))
            return Token::Tokens::TYPEDEF;

    case 'u':
        if (!keyword.compare("unsigned"))
            return Token::Tokens::UNSIGNED;

        else if (!keyword.compare("union"))
            return Token::Tokens::UNION;
        break;

    case 'v':
        if (!keyword.compare("void"))
            return Token::Tokens::VOID;
        else if (!keyword.compare("volatile"))
            return Token::Tokens::VOLATILE;
        break;

    case 'w':
        if (!keyword.compare("while"))
            return Token::Tokens::WHILE;
    case '_':
        if (!keyword.compare("__restrict"))
            return Token::Tokens::RESTRICT;
        else if (!keyword.compare("__attribute__"))
            return Token::Tokens::ATTRIBUTE;
        else if (!keyword.compare("__asm__"))
            return Token::Tokens::ASM;
    }
    return 0;
}
