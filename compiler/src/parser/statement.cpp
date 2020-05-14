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

struct ast_node *StatementParser::parseStatement(int parentTok)
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
    case Token::Tokens::L_BRACE:
        node = parseBlock(parentTok);
        break;
        
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
    
    case Token::Tokens::DO:
        DEBUGR("it never ran this right?")
        node = doWhileStatement();
        break;
    
    case Token::Tokens::SWITCH:
        node = switchStatement();
        break;
    
    case Token::Tokens::RETURN:
        node = returnStatement();
        break;
    case Token::Tokens::T_EOF:
        err.fatal("EOF read while block was not terminated with a '}'");

    case Token::Tokens::R_BRACE:
        break;
    
    case Token::Tokens::GOTO:
        node = gotoStatement();
        break;
    
    case Token::Tokens::CASE:
        if (parentTok == Token::Tokens::SWITCH)
            node = switchCaseStatement();
        else
            err.fatal("Case labels are only allowed inside switch statements");
        break;
    
    case Token::Tokens::DEFAULT:
        if (parentTok == Token::Tokens::SWITCH)
            node = switchDefaultStatement();
        else
            err.fatal("Default labels are only allowed inside switch statements");
        break;
    
    case Token::Tokens::CONTINUE:
        m_scanner.scan();
        node = mkAstLeaf(AST::Types::CONTINUE, 0, m_scanner.curLine(), 
                         m_scanner.curChar());
        break;
    
    case Token::Tokens::BREAK:
        m_scanner.scan();
        node = mkAstLeaf(AST::Types::BREAK, 0, m_scanner.curLine(), 
                         m_scanner.curChar());
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

    default:
        node = m_parser.m_exprParser.parseBinaryOperation(0, NULLTYPE);
        if (node)
            break;
    
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
        
        string ident = m_scanner.identifier();
        m_scanner.scan();
        if (m_scanner.token().token() == Token::Tokens::COLON)
        {
            DEBUGR("parsing label")
            node = parseLabel(ident);
            break;
        }
        
        err.unknownSymbol(ident);
    }

    if (node && node->operation != AST::Types::PADDING)
    {
        // Place statement strings into the asm files for debugging purposes
        //node = mkAstUnary(AST::Types::DEBUGPRINT, node, m_lastOffset+1, 
        //                  m_scanner.curLine(), m_scanner.curOffset()-1);
    }
    m_lastOffset = m_scanner.curOffset();
    return node;
}

bool isStatement(int op)
{
    if (op == AST::Types::FUNCTIONCALL || op == AST::Types::ASSIGN ||
        op == AST::Types::PADDING || op == AST::Types::RETURN ||
        op == AST::Types::INITIALIZER || op == AST::Types::LABEL ||
        op == AST::Types::BREAK || op == AST::Types::CONTINUE)
        return true;
        
    return false;
}

/// @brief  The actual parseblock function
struct ast_node *StatementParser::_parseBlock(int parentOp, struct ast_node *left)
{
    struct ast_node *tree = NULL;

    while (1)
    {
        tree = parseStatement(parentOp);

        if (tree && isStatement(tree->operation) && m_scanner.token().token() != Token::Tokens::R_BRACE)
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

struct ast_node *StatementParser::parseBlock(int parentOp, bool newScope)
{
    struct ast_node *tree = NULL;
    int id = -1;
    
    if (m_scanner.token().token() != Token::Tokens::L_BRACE)
    {
        tree = parseStatement(parentOp);
        int op = tree->operation;
        
        // Due to the debugprint ast nodes:
        if (op == AST::Types::DEBUGPRINT)
            op = tree->left->operation;
        
        if (isStatement(op))
            m_parser.match(Token::Tokens::SEMICOLON);
        
        return tree;
    }
    
    if (newScope)
    {
        id = g_symtable.newScope();
        DEBUGR("pushing scope: " << id)
        tree = mkAstLeaf(AST::Types::PUSHSCOPE, id, 0, 0);
    }
    
    m_scanner.scan();
    tree = _parseBlock(parentOp, tree);
    m_parser.match(Token::Tokens::R_BRACE);
    
    if (newScope)
    {
        DEBUGR("popping scope")
        g_symtable.popScope();
        struct ast_node *pop = mkAstLeaf(AST::Types::POPSCOPE, 0, 0, 0);
        tree = mkAstNode(AST::Types::GLUE, tree, NULL, pop, 0, 0, 0);
    }
    
    return tree;
}
