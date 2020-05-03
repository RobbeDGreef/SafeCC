#include <errorhandler.h>
#include <parser/parser.h>
#include <parser/statementparser.h>
#include <symbols.h>
#include <types.h>

Parser::Parser(Scanner &scanner, Generator &gen)
    : m_scanner(scanner), m_generator(gen),
      m_statementParser(scanner, *this, gen, m_typeList),
      m_exprParser(scanner, *this, gen, m_typeList)
{
    m_scanner = scanner;
}

struct Type Parser::parseType()
{
    struct Type t;
    int tok = m_scanner.token().token();
    switch (tok)
    {
    case Token::Tokens::CONST:
        m_scanner.scan();
        return parseType();
        
    case Token::Tokens::STRUCT:
    case Token::Tokens::UNION:
    case Token::Tokens::ENUM:
        m_scanner.scan();
        if (m_scanner.token().token() == Token::Tokens::L_BRACE)
        {
            if (tok == Token::Tokens::UNION)
                return m_statementParser.declUnion(0)->type;
            else if (tok == Token::Tokens::ENUM)
                return m_statementParser.declEnum()->type;
            else
                return m_statementParser.declStruct(0)->type;
        }
        
    case Token::Tokens::IDENTIFIER:
        t = m_typeList.getType(m_scanner.identifier());
        if (t.typeType == 0)
            return t;
        
        m_scanner.scan();

        // Check for pointer depth
        while (m_scanner.token().token() == Token::Tokens::STAR)
        {
            t.ptrDepth++;
            m_scanner.scan();
            t.size = PTRTYPE.size;
        }
        
        return t;

    default:
        vector<int> tokens = m_scanner.scanUntil(Token::Tokens::IDENTIFIER,
                                                 Token::Tokens::COMMA,
                                                 Token::Tokens::R_PAREN);
        return tokenToType(tokens);
    }
}

struct ast_node *Parser::parserMain()
{
    ast_node *tree = m_statementParser._parseBlock();
    match(Token::Tokens::T_EOF);
    return tree;
}

void Parser::match(int tok)
{
    if (m_scanner.token().token() == tok)
        m_scanner.scan();

    else
        err.expectedToken(tok, m_scanner.token().token());
}

void Parser::match(int tok, int tok2)
{
    if (m_scanner.token().token() == tok || m_scanner.token().token() == tok2)
        m_scanner.scan();

    else
        err.expectedToken(tok, tok2, m_scanner.token().token());
}

void Parser::match(int tok, int tok2, string error)
{
    if (m_scanner.token().token() == tok || m_scanner.token().token() == tok2)
        m_scanner.scan();

    else
        err.fatal(error);
}

void Parser::matchNoScan(int tok)
{
    if (m_scanner.token().token() != tok)
        err.expectedToken(tok, m_scanner.token().token());
}
