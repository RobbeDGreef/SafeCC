#include <symbols.h>
#include <scanner.h>
#include <core.h>
#include <token.h>
#include <errorhandler.h>

/// @brief  Returns a positive if the character is in the string, -1 otherwise
static int chrpos(string s, int c)
{
    char *p = strchr((char*) s.c_str(), c);
    return (p ? p - s.c_str() : -1);
}

/// @brief  Returns int value from a string (if possible)
int Scanner::scanint(int c)
{
    int i, val = 0;

    while ((i = chrpos("0123456789", c)) >= 0)
    {
        val = val * 10 + i;
        c = next();
    }

    putback(c);
    return val;
}

/// @brief  Returns the value from the hexadecimal number
int Scanner::scanhex(int c)
{
    int val = 0;
    int i = 0;
    while ((i = chrpos("0123456789abcdefABCDEF", c)) >= 0)
    {
        if (i > 15)
            i -= 6;

        val = val * 16 + i;
        c = next();
    }
    putback(c);
    return val;
}

int Scanner::scanoct(int c)
{
    int val = 0;
    int i = 0;
    while ((i = chrpos("01234567", c)) >= 0)
    {
        val = val * 8 + i;
        c = next();
    }
    putback(c);
    return val;
}

int Scanner::scanbin(int c)
{
    int val = 0;
    int i = 0;
    while ((i = chrpos("01", c)) >= 0)
    {
        val = val * 2 + i;
        c = next();
    }
    putback(c);
    return val;
}

int Scanner::scanIdentifier(int c)
{
    /* Used a global buffer instead of local to speed up (due to allocation and
     * deallocation of string space) */
    m_identBuf.clear();
    for (int i = 0; i < SCANNER_IDENTIFIER_LIMMIT; i++)
    {
        if (isalpha(c) || isdigit(c) || c == '_')
        {
            m_identBuf += c;
            c = next();
        }
        else
        {
            putback(c);
            return i;
        }
    }
    err.syntaxError("identifier too long");
}

int Scanner::scanChar()
{
    int c = next();
    c = charParser(c);
    if (next() != '\'')
        err.fatal("Expected end of character literal");

    return c;
}

int Scanner::scanStringLiteral()
{
    int c = 0;
    string end = "";

    while ((c = next()) != '"')
    {
        if (c == EOF)
            err.fatal("Read end of file before string was terminated");
        
        c = charParser(c);
        
        end += c;
    }

    return g_symtable.addString(end);
}

/// @brief  The actual scan function, will scan the input stream for tokens
int Scanner::scan()
{
    if (m_putbackToken.token() != -1)
    {
        m_token.set(m_putbackToken.token(), m_putbackToken.intValue(), m_line, m_char);
        m_putbackToken.set(-1, m_line, m_char);
        return 1;
    }

    int c;
    c = skip();

    switch (c)
    {
    case EOF:
        m_token.set(Token::Tokens::T_EOF, m_line, m_char);
        return 0;

    case '+':
        c = next();
        if (c == '+')
            m_token.set(Token::Tokens::INC, m_line, m_char);
        else
        {
            m_token.set(Token::Tokens::PLUS, m_line, m_char);
            putback(c);
        }
        break;

    case '-':
        c = next();
        if (c == '-')
            m_token.set(Token::Tokens::DEC, m_line, m_char);
        else
        {
            m_token.set(Token::Tokens::MINUS, m_line, m_char);
            putback(c);
        }
        break;

    case '*':
        m_token.set(Token::Tokens::STAR, m_line, m_char);
        break;

    case '/':
        m_token.set(Token::Tokens::SLASH, m_line, m_char);
        break;

    case ';':
        m_token.set(Token::Tokens::SEMICOLON, m_line, m_char);
        break;

    case '=':
        c = next();
        if (c == '=')
            m_token.set(Token::Tokens::EQUAL, m_line, m_char);

        else
        {
            putback(c);
            m_token.set(Token::Tokens::EQUALSIGN, m_line, m_char);
        }

        break;

    case '<':
        c = next();
        if (c == '=')
            m_token.set(Token::Tokens::LESSTHANEQUAL, m_line, m_char);
        else if (c == '<')
            m_token.set(Token::Tokens::L_SHIFT, m_line, m_char);
        else
        {
            putback(c);
            m_token.set(Token::Tokens::LESSTHAN, m_line, m_char);
        }
        break;

    case '>':
        c = next();
        if (c == '=')
            m_token.set(Token::Tokens::GREATERTHANEQUAL, m_line, m_char);
        else if (c == '>')
            m_token.set(Token::Tokens::R_SHIFT, m_line, m_char);
        
        else
        {
            putback(c);
            m_token.set(Token::Tokens::GREATERTHAN, m_line, m_char);
        }
        break;

    case '!':
        c = next();
        if (c == '=')
            m_token.set(Token::Tokens::NOTEQUAL, m_line, m_char);
        else
        {
            putback(c);
            m_token.set(Token::Tokens::LOGNOT, m_line, m_char);
        }
        break;

    case '{':
        m_token.set(Token::Tokens::L_BRACE, m_line, m_char);
        break;

    case '}':
        m_token.set(Token::Tokens::R_BRACE, m_line, m_char);
        break;

    case '(':
        m_token.set(Token::Tokens::L_PAREN, m_line, m_char);
        break;

    case ')':
        m_token.set(Token::Tokens::R_PAREN, m_line, m_char);
        break;

    case ',':
        m_token.set(Token::Tokens::COMMA, m_line, m_char);
        break;

    case '&':
        c = next();
        if (c == '&')
            m_token.set(Token::Tokens::LOGAND, m_line, m_char);
        
        else
        {
            m_token.set(Token::Tokens::AMPERSANT, m_line, m_char);
            putback(c);
        }
        break;

    case '[':
        m_token.set(Token::Tokens::L_BRACKET, m_line, m_char);
        break;

    case ']':
        m_token.set(Token::Tokens::R_BRACKET, m_line, m_char);
        break;

    case '"':
        m_token.set(Token::Tokens::STRINGLIT, scanStringLiteral(), m_line, m_char);
        break;

    case '\'':
        m_token.set(Token::Tokens::INTLIT, scanChar(), m_line, m_char);
        break;
    case '.':
        m_token.set(Token::Tokens::DOT, m_line, m_char);
        break;
    
    case '|':
        c = next();
        
        if (c == '|')
            m_token.set(Token::Token::LOGOR, m_line, m_char);
        else
        {
            m_token.set(Token::Tokens::OR, m_line, m_char);
            putback(c);
        }
        
        break;
    
    case '^':
        m_token.set(Token::Tokens::XOR, m_line, m_char);
        break;
    
    case '~':
        m_token.set(Token::Tokens::TIDDLE, m_line, m_char);
        break;
    
    case '%':
        m_token.set(Token::Tokens::MODULUS, m_line, m_char);
        break;
    
    case ':':
        m_token.set(Token::Tokens::COLON, m_line, m_char);
        break;
    
    case '?':
        m_token.set(Token::Tokens::QUESTIONMARK, m_line, m_char);
        break;
    
    case '0':
        /* checking whether it is an octal, hexadecimal or binary number */
        c = next();
        if (c == 'x')
        {
            /* Hexadecimal number */
            m_token.set(Token::Token::INTLIT, scanhex(next()), m_line, m_char);
        }
        else if (c == 'b')
        {
            /* Binary number */
            m_token.set(Token::Token::INTLIT, scanbin(next()), m_line, m_char);
        }
        else
        {
            /* Octal number */
            m_token.set(Token::Tokens::INTLIT, scanoct(c), m_line, m_char);
        }
        break;

    default:
        if (isdigit(c))
        {
            m_token.set(Token::Tokens::INTLIT, scanint(c), m_line, m_char);
            break;
        }
        else if (isalpha(c) || c == '_')
        {
            int col = m_char;
            int line = m_line;
            scanIdentifier(c);
            
            int token = identifyKeyword(m_identBuf);
            if (token)
            {
                m_token.set(token, line, col);
                
                // Whitespace implmented keywords
                if (token == Token::Tokens::RESTRICT)
                {
                    scan();
                }
                
                break;
            }

            /* unrecognized keyword, must be identifier */
            m_token.set(Token::Tokens::IDENTIFIER, line, col);
            break;
        }

        err.fatal("Unrecognized character '" + string(1, (char)c) + "'");
    }
    
    //err.loadErrorInfo(err.createErrorInfo());
    //err.unloadErrorInfo();
    
    return 1;
}
