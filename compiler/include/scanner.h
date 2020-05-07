#pragma once

#include <token.h>
#include <core.h>

#define SCANNER_IDENTIFIER_LIMMIT   100

enum Identifiers
{
    PRINT = 1
};

/**
 * @brief   Scanner class implemented the lexical scanner of the compiler
 *  
 */
class Scanner
{

private:
    int         m_line = 1;     // Line numbers start at 1 (you know because normal people start counting from 1)
    int         m_char = 0;

    Token       m_token;
    Token       m_putbackToken;

    int         m_putback = 0;
    
    string      m_identBuf;
    string      m_ppFile;

    const char  *m_filename;
    FILE        *m_infile;
    
private:
    int next();
    int skip();
    int skipLine();
    int skipMultiLineComment();
    void putback(int c);
    int scanint(int c);
    int scanhex(int c);
    int scanoct(int c);
    int scanbin(int c);
    int scanStringLiteral();
    int scanChar();
    int charParser(int c);
    int scanIdentifier(int c);
    int identifyKeyword(string keyword);
    int parsePPStatement();

public:
    Scanner(const char *path);
    ~Scanner();

    Token& token();
    int curLine();
    int curChar();
    int curOffset();
    string curPPFile();
    string curFunction();
    string curStrLine(int offset);
    string &identifier();
    int getTokenStart();

    int scan();
    void putbackToken(Token t);
    vector<int> scanUntil(int tok);
    vector<int> scanUntil(int tok, int tok2, int tok3);


};