#include <token.h>
#include <core.h>


int Token::token()
{
    return m_token;
}

int Token::intValue()
{
    return m_intValue;
}

void Token::set(int tok, int line, int col)
{
    m_startCol = m_endCol;
    m_endCol = col;
    
    m_startLine = m_endLine;
    m_endLine = line;
    
    m_token = tok;
}

void Token::set(int tok, int val, int line, int col)
{
    m_intValue = val;
    set(tok, line, col);
}


string tokToStr(int token)
{
    return toknames[token] + " (" + to_string(token) + ")";
}

int Token::startLine()
{
    return m_startLine;
}

int Token::startCol()
{
    return m_startCol;    
}

int Token::endLine()
{
    return m_endLine;
}

int Token::endCol()
{
    return m_endCol;
}

int isArithmetic(int tok)
{
    if (tok == Token::Tokens::PLUS ||
        tok == Token::Tokens::MINUS ||
        tok == Token::Tokens::STAR ||
        tok == Token::Tokens::SLASH ||
        tok == Token::Tokens::MODULUS ||
        tok == Token::Tokens::INC ||
        tok == Token::Tokens::DEC)
    {
        return true;
    }
    
    return false;
}