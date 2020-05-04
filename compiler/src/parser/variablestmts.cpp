#include <config.h>
#include <errorhandler.h>
#include <parser/parser.h>
#include <symbols.h>
#include <token.h>

struct ast_node *StatementParser::parseArrayInit(struct Type   type,
                                                 struct Symbol sym)
{
    int              i;
    int              tok;
    struct ast_node *tree  = NULL;
    struct ast_node *right = NULL;

    // The value (symbol id) will be set at the end of the function
    struct ast_node *ident = mkAstLeaf(AST::Types::IDENTIFIER, 0, sym.varType,
                                       m_scanner.curLine(),
                                       m_scanner.curChar());

    if (m_scanner.token().token() == Token::Tokens::L_BRACE)
    {
        m_scanner.scan();

        int arraySize;

        if (sym.value == -1)
        {
            arraySize = MAX_ARRAY_INIT;
            sym.value = 0;
        }
        else
            arraySize = sym.value;

        for (i = 0; i < arraySize; i++)
        {
            if (arraySize != MAX_ARRAY_INIT && i > sym.value)
            {
                err.warning("Initializer overflow");
                continue;
            }

            right = m_parser.m_exprParser.parseBinaryOperation(0, type);

            if (g_symtable.isCurrentScopeGlobal())
            {
                if (right->operation == AST::Types::INTLIT)
                    sym.inits.push_back(to_string(right->value));

                else if (right->operation == AST::Types::IDENTIFIER)
                    sym.inits.push_back(g_symtable.getSymbol(right->value)->name);

                else
                    err.fatal("Global array does not support initializer");
            }
            else
            {
                tree = mkAstNode(AST::Types::DIRECTMEMLOAD, tree, ident, right,
                                 i, type, m_scanner.curLine(),
                                 m_scanner.curChar());
            }

            if (arraySize == MAX_ARRAY_INIT)
                sym.value++;

            tok = m_scanner.token().token();
            m_scanner.scan();
            if (tok == Token::Tokens::COMMA)
                continue;
            else if (tok == Token::Tokens::R_BRACE)
                break;
            else
                err.unexpectedToken(Token::Tokens::COMMA);
        }

        if (tok == Token::Tokens::COMMA)
            err.fatal("Initializers overflow, expected " +
                      to_string(arraySize) + " initializers");

        // Reversing the array in memory
        struct ast_node *tmp = tree;

        while (tmp != NULL)
        {
            tmp->value = (sym.value - 1 - tmp->value) * (tmp->type.size / 8);
            tmp        = tmp->left;
        }

        int id       = g_symtable.pushSymbol(sym);
        ident->value = id;
    }
    else if (m_scanner.token().token() == Token::Tokens::STRINGLIT)
    {
        right = m_parser.m_exprParser.parseBinaryOperation(0, sym.varType);
        struct Symbol *init = g_symtable.getSymbol(right->value);

        if (sym.value == -1)
            sym.value = init->value;

        else if (sym.value < init->value)
            err.warning("String overflows array initializers");

        // Just reset the string name etc
        init->name    = sym.name;
        init->varType = sym.varType;
        init->value   = sym.value;
    }
    else
        err.fatal("Invalid initializer");

    return mkAstUnary(AST::Types::ASSIGN, tree, 0, m_scanner.curLine(),
                      m_scanner.curChar());
}

struct ast_node *StatementParser::parseVarInit(struct Type   type,
                                               struct Symbol sym)
{
    // Apparently the ISO c standard has no problem with this
    #if 0
    if (m_scanner.token().token() == Token::Tokens::STRINGLIT ||
        m_scanner.token().token() == Token::Tokens::L_BRACE)
    {
        err.fatal("Invalid initializer")
    }
    
    #endif
    
    struct ast_node *left = m_parser.m_exprParser.parseBinaryOperation(0, type);

    if (left->operation == AST::Types::INTLIT)
    {
        if (g_symtable.isCurrentScopeGlobal())
        {
            // Simply set it as a initialiser value in the symbol table
            sym.value = left->value;
            g_symtable.pushSymbol(sym);
            return mkAstLeaf(AST::Types::PADDING, 0, type, 0, 0);
        }
    }

    int              id   = g_symtable.pushSymbol(sym);
    struct ast_node *tree = mkAstLeaf(AST::Types::LOADLOCATION, id, type,
                                      left->line, left->c);

    tree = mkAstUnary(AST::Types::LEFTVALIDENT, tree, 0, type, left->line,
                      left->c);

    return mkAstNode(AST::Types::ASSIGN, left, NULL, tree, 0, type, left->line,
                     left->c);
}

struct ast_node *StatementParser::variableDecl(struct Type type, int sc)
{
    /**
     * possible declarations:
     *
     * int x;
     * int x = y;
     * int *x;
     * int *x = y;
     * int x[] = {y};
     * int x[5] = {y, ...};
     *
     */

    struct Symbol s;
    s.name         = m_scanner.identifier();
    s.varType      = type;
    s.storageClass = sc;
    s.used = false;
    s.defined = false;

    if (g_symtable.findInCurrentScope(s.name) != -1)
        err.fatal("Redefinition of symbol " + HL("'" + s.name + "'"));

    if (m_scanner.token().token() == Token::Tokens::L_BRACKET)
    {
        /* Initialising array so next token must be integer or ] */
        m_scanner.scan();
        s.varType.ptrDepth++;
        s.varType.size = DWORD;
        s.symType = SymbolTable::SymTypes::VARIABLE;
        s.varType.isArray = true;

        if (m_scanner.token().token() == Token::Tokens::INTLIT)
        {
            /* Specific size */
            s.value   = m_scanner.token().intValue();

            m_scanner.scan();
        }
        else if (m_scanner.token().token() == Token::Tokens::R_BRACKET)
        {
            /* Let the compiler count size */
            s.value   = -1;
        }
        else
            err.fatal("Expected non negative integer constant or nothing at "
                      "all");

        m_parser.match(Token::Tokens::R_BRACKET);
    }
    else
    {
        s.symType = SymbolTable::VARIABLE;
        s.value   = 0;
    }

    if (m_scanner.token().token() == Token::Tokens::SEMICOLON)
    {
        if (s.varType.isArray && s.value == -1 && 
            sc != SymbolTable::StorageClass::EXTERN)
            err.fatal("Cannot declare array without size specifier and "
                      "initialiser");

        g_symtable.pushSymbol(s);

        return mkAstLeaf(AST::Types::PADDING, 0, type, 0, 0);
    }
    else
    {
        m_parser.match(Token::Tokens::EQUALSIGN);

        s.defined = true;

        if (sc == SymbolTable::StorageClass::EXTERN &&
            !g_symtable.isCurrentScopeGlobal())
            err.fatal("Has both 'extern' and initializer");

        else if (sc == SymbolTable::StorageClass::EXTERN)
            err.warning("Has both 'extern' and initializer");

        if (s.varType.isArray)
        {
            // Array initialiser
            return parseArrayInit(type, s);
        }
        else if (s.varType.typeType == TypeTypes::STRUCT && !s.varType.ptrDepth)
        {
            return parseStructInit(type, s);
        }
        else
        {
            // Regular initialiser
            return parseVarInit(type, s);
        }
    }
}
/////////////////////////////////////////////////////////
/// @brief Generates a variable assignment ast tree
///
/// @param ptrDepth the depth of the pointer
/// @return struct ast_node* ast tree
///
/// The tree generated here will be:
///        |
///      ASSIGN
///      /    \
///  primary  lvalident
///             /
///     (ptr accesses may happen depending on ptr depth)
///           /
///     loadaddress (of var)
///
///
struct ast_node *StatementParser::variableAssignment(int ptrDepth)
{
    int id = g_symtable.findSymbol(m_scanner.identifier());

    if (id == -1)
        err.fatal("Undeclared variable: '" + m_scanner.identifier() + "'");

    struct Type type = g_symtable.getSymbol(id)->varType;

    struct ast_node *right = mkAstLeaf(AST::Types::LOADLOCATION, id, type,
                                       m_scanner.curLine(),
                                       m_scanner.curChar());
    if (type.typeType == TypeTypes::STRUCT && type.ptrDepth)
        right = mkAstUnary(AST::Types::PTRACCESS, right, 0, type, right->line,
                           right->c);

    right = m_parser.m_exprParser.parsePostfixOperator(right, false);

    type = right->type;

    /* Check if pointer depth is correct */
    if (ptrDepth > type.ptrDepth)
        err.fatal("Variable '" + g_symtable.getSymbol(id)->name +
                  "' does not have pointerdepth " + to_string(ptrDepth));

    for (int i = 0; i < ptrDepth; i++)
    {
        dereference(&type);
        right = mkAstUnary(AST::Types::PTRACCESS, right, 0, type, right->line,
                           right->c);
    }

    struct ast_node *left = NULL;

    right = mkAstUnary(AST::Types::LEFTVALIDENT, right, 0, type,
                       m_scanner.curLine(), m_scanner.curChar());

    m_parser.match(Token::Tokens::EQUALSIGN);

    left = m_parser.m_exprParser.parseBinaryOperation(0, type);

    struct ast_node *tree = mkAstNode(AST::Types::ASSIGN, left, NULL, right, 0,
                                      type, m_scanner.curLine(),
                                      m_scanner.curChar());
    return tree;
}