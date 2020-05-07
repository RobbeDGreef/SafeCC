#include <errorhandler.h>
#include <parser/parser.h>
#include <symbols.h>
#include <types.h>

StatementParser::StatementParser(Scanner &scanner, Parser &parser,
                                 Generator &gen, TypeList &typelist)
    : m_scanner(scanner), m_parser(parser), m_generator(gen),
      m_typeList(typelist)
{
}

struct ast_node *StatementParser::parseDeclaration(struct Type type, int sc)
{

    m_parser.match(Token::Tokens::IDENTIFIER);

    if (m_scanner.token().token() == Token::Tokens::L_PAREN)
    {
        if (sc == -1)
            sc = SymbolTable::StorageClass::EXTERN;
        return functionDecl(type, sc);
    }
    else
    {
        if (sc == -1)
            sc == SymbolTable::StorageClass::AUTO;
        return variableDecl(type, sc);
    }
}

struct ast_node *StatementParser::parseTypedef()
{
    m_scanner.scan();
    struct Type t = m_parser.parseType();
    if (t.typeType == 0)
        err.unknownType(m_scanner.identifier());

    t.name = new string(m_scanner.identifier());
    m_typeList.addType(t);

    m_scanner.scan();

    // I don't know why this is allowed in c, but the linux clib headers
    // have like these kinds of things in them
    // typedef __ssize_t __io_write_fn (void *__cookie, const char *__buf,
    // size_t __n); and i have no clue what they are
    if (m_scanner.token().token() == Token::Tokens::L_PAREN)
    {
        err.warning("functionpointer typedefs are unimplemented");
        m_scanner.scanUntil(Token::Tokens::SEMICOLON);
    }

    return mkAstLeaf(AST::PADDING, 0, 0, 0);
}

struct ast_node *StatementParser::parseStatement()
{
    int              ptrOcc       = 0;
    int              storageClass = -1;
    struct ErrorInfo errinfo;
    struct ast_node *node = NULL;

loop:;
    int         tok = m_scanner.token().token();
    struct Type type = NULLTYPE;
    string      ident;

    switch (tok)
    {
    case Token::Tokens::EXTERN:
    case Token::Tokens::STATIC:
    case Token::Tokens::REGISTER:
    case Token::Tokens::AUTO:
        storageClass = tok - Token::Tokens::AUTO;
        m_scanner.scan();
        goto loop;
    
    case Token::Tokens::IF:
        node = ifStatement();
        break;

    case Token::Tokens::TYPEDEF:
        node = parseTypedef();
        break;
        
    case Token::Tokens::WHILE:
        node = whileStatement();
        break;
        
    case Token::Tokens::FOR:
        node = forStatement();
        break;
        
    case Token::Tokens::RETURN:
        node = returnStatement();
        break;
    case Token::Tokens::T_EOF:
        err.fatal("EOF read while block was not terminated with a '}'");

    case Token::Tokens::R_BRACE:
        break;

    case Token::Tokens::SIGNED:
    case Token::Tokens::UNSIGNED:
    case Token::Tokens::VOID:
    case Token::Tokens::CHAR:
    case Token::Tokens::SHORT:
    case Token::Tokens::INT:
    case Token::Tokens::LONG:
    case Token::Tokens::STRUCT:
    case Token::Tokens::UNION:
    case Token::Tokens::ENUM:
    case Token::Tokens::CONST:
        type = m_parser.parseType();
        if (type.typeType == 0 ||
            m_scanner.token().token() == Token::Tokens::SEMICOLON ||
            m_scanner.token().token() == Token::Tokens::L_BRACE)
        {
            if (tok == Token::Tokens::STRUCT)
            {
                node = declStruct(storageClass);
                break;
            }
            else if (tok == Token::Tokens::UNION)
            {
                node = declUnion(storageClass);
                break;
            }
            else if (tok == Token::Tokens::ENUM)
            {
                node = declEnum();
                break;
            }

            err.unknownType(m_scanner.identifier());
        }

        if (type.typeType == TypeTypes::ENUM)
            type.typeType = TypeTypes::VARIABLE;

        node = parseDeclaration(type, storageClass);
        break;

    case Token::Tokens::IDENTIFIER:
        type = m_parser.parseType();

        if (type.typeType != 0)
        {
            while (m_scanner.token().token() == Token::Tokens::STAR)
            {
                ptrOcc++;
                m_scanner.scan();
            }
            type.ptrDepth += ptrOcc;
            node = parseDeclaration(type, storageClass);
            break;
        }

    default:
        node = m_parser.m_exprParser.parseBinaryOperation(0, NULLTYPE);
        break;
    }

    if (node && node->operation != AST::Types::PADDING)
    {
        // Place statement strings into the asm files for debugging purposes
        node = mkAstUnary(AST::Types::DEBUGPRINT, node, m_lastOffset+1, 
                          m_scanner.curLine(), m_scanner.curOffset()-1);
    }
    m_lastOffset = m_scanner.curOffset();
    return node;
}

bool isStatement(int op)
{
    if (op == AST::Types::FUNCTIONCALL || op == AST::Types::ASSIGN ||
        op == AST::Types::PADDING || op == AST::Types::RETURN ||
        op == AST::Types::INITIALIZER)
        return true;
        
    return false;
}

/// @brief  The actual parseblock function
struct ast_node *StatementParser::_parseBlock()
{
    struct ast_node *tree = NULL;
    struct ast_node *left = NULL;

    while (1)
    {
        tree = parseStatement();

        if (tree && (tree->operation == AST::Types::FUNCTIONCALL ||
                     tree->operation == AST::Types::ASSIGN ||
                     tree->operation == AST::Types::PADDING ||
                     tree->operation == AST::Types::RETURN))
        {
            /* Only some statements need a semicolon at the end */
            m_parser.match(Token::Tokens::SEMICOLON);
        }

        // Padding nodes shouldn't be added to the ast tree
        if (tree && tree->operation == AST::Types::PADDING)
        {
            delete tree;
            tree = NULL;
        }

        if (tree)
        {
            if (left == NULL)
                left = tree;
            else
                left = mkAstNode(AST::Types::GLUE, left, NULL, tree, 0,
                                 m_scanner.curLine(), m_scanner.curChar());
        }

        /* Hit right brace so return left */
        if (m_scanner.token().token() == Token::Tokens::R_BRACE ||
            m_scanner.token().token() == Token::Tokens::T_EOF)
        {
            return left;
        }
    }
}

struct ast_node *StatementParser::parseBlock(vector<struct Symbol> arguments)
{
    /* Each block should start with a { */
    m_parser.match(Token::Tokens::L_BRACE);

    g_symtable.newScope();

    for (struct Symbol s : arguments)
    {
        g_symtable.pushSymbol(s);
    }

    struct ast_node *tree = _parseBlock();

    m_parser.match(Token::Tokens::R_BRACE);
    return tree;
}

struct ast_node *StatementParser::parseBlock()
{
    /* Each block should start with a { */
    m_parser.match(Token::Tokens::L_BRACE);

    g_symtable.newScope();
    struct ast_node *tree = _parseBlock();
    m_parser.match(Token::Tokens::R_BRACE);
    g_symtable.popScope();
    return tree;
}
