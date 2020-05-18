#include <core.h>
#include <errorhandler.h>
#include <scanner.h>
#include <symbols.h>

/// @brief   Opens the given file, if it can't it will throw an error
Scanner::Scanner(const char *filename)
{
    m_infile = fopen(filename, "r");
    m_filename = filename;

    if (m_infile == NULL)
        err.fatalNL("Unable to open '" + string(filename) + "'");

    /* Init ident buffer */
    m_identBuf.reserve(SCANNER_IDENTIFIER_LIMMIT + 1);
    m_putbackToken.set(-1, m_line, m_char);
}

/// @brief  Close the opened file
Scanner::~Scanner()
{
    fclose(m_infile);
}

int Scanner::curLine()
{
    return m_line;
}

int Scanner::curChar()
{
    return m_char;
}

Token &Scanner::token()
{
    return m_token;
}

string &Scanner::identifier()
{
    return m_identBuf;
}

void Scanner::putback(int c)
{
    m_putback = c;
}

int Scanner::peek()
{
    Token tok = m_token;
    scan();
    int peek = m_token.token();
    putbackToken(m_token.token());
    m_token = tok;
    return peek;
}

/// @brief  Returns next (usefull) character from the input stream
int Scanner::next()
{
    int c;

    if (m_putback)
    {
        c = m_putback;
        m_putback = 0;
        return c;
    }

    c = fgetc(m_infile);
    if ('\n' == c)
    {
        m_line++;
        m_char = 0;
    }
    else
        m_char++;
    
    return c;
}

int Scanner::skipLine()
{
    int c;
    while ((c = next()) != '\n');
    return next();
}

int Scanner::skipMultiLineComment()
{
    int c;
    while ((c = next()) != EOF)
    {
        if (c == '*')
            if ((c = next()) == '/')
                return next();
    }

    err.fatal("End of file read before end of multi line comment");
}

/// @brief  Skips over useless whitespace / comments
int Scanner::skip()
{
    int c;

    c = next();

    while (c == '/' || c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '#')
    {
        if (c == '/')
        {
            c = next();
            if (c == '/')
                c = skipLine();
            else if (c == '*')
                c = skipMultiLineComment();
            else
            {
                putback(c);
                return '/';
            }
        }
        else if (c == '#')
            c = parsePPStatement();
        else
            c = next();
    }

    return c;
}

vector<int> Scanner::scanUntil(int tok)
{
    /* @todo: this is dangerous because T_EOF = 0 */
    return scanUntil(tok, 0, 0);
}

vector<int> Scanner::scanUntil(int tok1, int tok2, int tok3)
{
    vector<int> tokens;
    int parenDepth = 0;
    
    for (int i = 0; i < MAX_SCAN_UNTIL_TOKENS; i++)
    {
        if (m_token.token() == Token::Tokens::L_PAREN)
            parenDepth++;
        
        else if (m_token.token() == Token::Tokens::R_PAREN)
            parenDepth--;
            
        tokens.push_back(m_token.token());
        scan();
        if (m_token.token() == tok1 || m_token.token() == tok2 ||
            m_token.token() == tok3 && parenDepth == 0)
            return tokens;
    }

    err.fatal("Infinite token scan loop detected");
}

int Scanner::charParser(int c)
{
    if (c == '\\')
    {
        c = next();
        switch (c)
        {
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            case '"': return '"';
            case '\'': return '\'';
            case '\\': return '\\';
        }
    }
    return c;
}

void Scanner::putbackToken(Token t)
{
    m_putbackToken = t;
}

int Scanner::parsePPStatement()
{
    int c = next();
    scan();
    
    if (m_token.token() != Token::Tokens::INTLIT)
    {
        err.warning("Incorrect preprocessor statement");
        return skipLine();
    }

    m_line = m_token.intValue() - 1;
    
    c = skip();
    if (c != '"')
    {
        err.warning("Incorrect preprocessor statement");
        return skipLine();
    }
    
    c = skip();
    if (c == '<')
        return skipLine();
    
    string buf = "";
    while (c != '"')
    {
        buf += c;
        c = next();
    }
    
    m_ppFile = buf;
    
    return skipLine();
}

string Scanner::curPPFile()
{
    return m_ppFile;
}

string Scanner::curFunction()
{
    if (g_symtable.currentFuncIdx() == -1)
        return "";
    return g_symtable.getSymbol(g_symtable.currentFuncIdx())->name;
}

string Scanner::curStrLine(int loc)
{
    unsigned long pos = ftell(m_infile);
    
    string str = "";
    fseek(m_infile, loc, SEEK_SET);
    int c = fgetc(m_infile);
    while (c != '\n')
    {
        str += c;
        c = fgetc(m_infile);
    }
        
    fseek(m_infile, pos, SEEK_SET);
    
    return str;
}

int Scanner::curOffset()
{
    return ftell(m_infile);
}

string Scanner::getStrFromTo(int from, int to)
{
    unsigned long pos = ftell(m_infile);
    
    string str = "";
    fseek(m_infile, from, SEEK_SET);
    
    for (int i = from; i <= to; i++)
    {
        str += fgetc(m_infile);
    }
    
    fseek(m_infile, pos, SEEK_SET);
    
    return str;
}


void Scanner::setIdentifier(string s)
{
    m_identBuf = s;
}