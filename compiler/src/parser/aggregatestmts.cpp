#include <config.h>
#include <core.h>
#include <errorhandler.h>
#include <parser/parser.h>
#include <parser/statementparser.h>
#include <symbols.h>

int anonStructCount = 0;

struct ast_node *StatementParser::structInit(struct ast_node *tree,
                                             struct ast_node *ident,
                                             struct ast_node *right,
                                             struct Symbol *sym, int idx)
{
    int offset = sym->varType.size - sym->varType.contents[idx].offset -
                 sym->varType.contents[idx].itemType.size / 8;

    if (g_symtable.isCurrentScopeGlobal())
    {
        if (right->operation == AST::Types::INTLIT)
            sym->inits.push_back(to_string(right->value));
        else if (right->operation == AST::Types::IDENTIFIER)
            sym->inits.push_back(g_symtable.getSymbol(right->value)->name);
        else
            err.fatal("Global array does not support initializer");
    }
    else
    {
        tree = mkAstNode(AST::Types::DIRECTMEMLOAD, tree, ident, right, offset,
                         sym->varType.contents[idx].itemType,
                         m_scanner.curLine(), m_scanner.curChar());
    }

    return tree;
}

struct ast_node *StatementParser::parseStructInit(struct Type   structType,
                                                  struct Symbol sym)
{
    DEBUG("struct init")
    struct ast_node *tree    = NULL;
    struct ast_node *left    = NULL;
    struct ast_node *right   = NULL;
    struct ast_node *expr    = NULL;
    struct ast_node *offNode = NULL;
    struct ast_node *tmp     = NULL;
    struct Type      argtype;
    struct ast_node *ident = mkAstLeaf(AST::Types::IDENTIFIER, 0, structType,
                                       m_scanner.curLine(),
                                       m_scanner.curChar());

    if (m_scanner.token().token() != Token::Tokens::L_BRACE)
    {
        left = m_parser.m_exprParser.parseBinaryOperation(0, structType);

        if (left->operation == AST::Types::STRUCTDEREFERENCE)
            left = left->left;

        if (left->type.typeType != STRUCT ||
            left->type.name->compare(*structType.name))
            err.expectedType(&structType, &left->type);

        int l = m_scanner.curLine();
        int c = m_scanner.curChar();

        tree = mkAstUnary(AST::Types::LEFTVALIDENT, ident, 0, structType, l, c);
    }
    else
    {
        m_scanner.scan();
        if (m_scanner.token().token() == Token::Tokens::DOT)
        {
            // designated initializer
            bool *inits = new bool[sym.varType.contents.size()];
            memset(inits, 0, sizeof(bool) * sym.varType.contents.size());

            int i;
            for (i = 0; i < MAX_STRUCT_DESIGNATED_INIT; i++)
            {
                m_parser.match(Token::Tokens::DOT);
                m_parser.match(Token::Tokens::IDENTIFIER);

                int itemIdx = findStructItem(m_scanner.identifier(),
                                             sym.varType);

                if (inits[itemIdx])
                    err.warning("Overwritten previously assigned initializer");
                else
                    inits[itemIdx] = true;

                m_parser.match(Token::Tokens::EQUALSIGN);

                argtype = sym.varType.contents[itemIdx].itemType;
                right = m_parser.m_exprParser.parseBinaryOperation(0, argtype);
                tree  = structInit(tree, ident, right, &sym, itemIdx);

                if (m_scanner.token().token() == Token::Tokens::R_BRACE)
                    break;

                m_parser.match(Token::Tokens::COMMA);
            }

            delete[] inits;

            if (i == MAX_STRUCT_DESIGNATED_INIT)
                err.fatal("Maximum designated initialsers is " +
                          to_string(MAX_STRUCT_DESIGNATED_INIT));
        }
        else
        {
            for (int i = 0; i < sym.varType.contents.size(); i++)
            {
                argtype = sym.varType.contents[i].itemType;
                right = m_parser.m_exprParser.parseBinaryOperation(0, argtype);
                tree  = structInit(tree, ident, right, &sym, i);

                if (m_scanner.token().token() == Token::Tokens::R_BRACE)
                    break;

                m_parser.match(Token::Tokens::COMMA);
            }

            if (m_scanner.token().token() != Token::Tokens::R_BRACE)
                err.fatal("Struct initialiser overflow");
        }
        m_scanner.scan();
    }

    int id       = g_symtable.pushSymbol(sym);
    ident->value = id;

    DEBUG("leaving")
    return mkAstNode(AST::Types::ASSIGN, left, NULL, tree, 0,
                     m_scanner.curLine(), m_scanner.curChar());
}

struct ast_node *StatementParser::declStruct(int storageClass)
{
    struct Type structType;
    structType.primType   = 0;
    structType.ptrDepth   = 0;
    structType.incomplete = false;
    structType.typeType   = TypeTypes::STRUCT;
    structType.isArray    = false;
    bool   redecl         = false;
    string s              = "anonymous" + to_string(anonStructCount++);

    if (m_scanner.token().token() == Token::Tokens::IDENTIFIER)
    {
        s = m_scanner.identifier();
        m_scanner.scan();
    }

    structType.name = new string(s);

    if (s.compare("anonymous" + to_string(anonStructCount-1)) &&
        m_typeList.getType(*structType.name).typeType != 0)
    {
        DEBUG("redecl of: " << s)
        redecl = true;

        if (!m_typeList.getType(*structType.name).incomplete)
            err.fatal("trying to initialise an already initialized struct");
    }

    if (m_scanner.token().token() == Token::Tokens::SEMICOLON)
    {
        // Forward declare
        structType.incomplete = true;

        // Double forward declare ??
        if (!redecl)
            m_typeList.addType(structType);

        return mkAstLeaf(AST::Types::PADDING, 0, 0, 0);
    }
    
    if (!redecl)
        m_typeList.addType(structType);

    m_parser.match(Token::Tokens::L_BRACE);

    int offset = 0;

    struct StructItem sItem;

    if (m_scanner.token().token() == Token::Token::R_BRACE)
        goto noItems;

    int i;
    for (i = 0; i < STRUCT_MAX_ITEMS; i++)
    {
        sItem.itemType = m_parser.parseType();
        if (sItem.itemType.typeType == 0)
            err.unknownType(&sItem.itemType);

        m_parser.match(Token::Tokens::IDENTIFIER);
        sItem.name = m_scanner.identifier();

        
        switch (sItem.itemType.size)
        {
        case BYTE:
            sItem.offset = offset;
            break;

        case WORD:
            if (offset % 2)
                offset += offset % 2;

            sItem.offset = offset;
            break;

        case DWORD:
            if (offset % 4)
                offset += 4 - (offset % 4);

            sItem.offset = offset;
            break;

        case QWORD:
            if (offset % 4)
                offset += 4 - (offset % 4);

            sItem.offset = offset;
            break;
        }
        
        if (m_scanner.token().token() == Token::Tokens::L_BRACKET)
        {
            m_scanner.scan();
            int num = m_parser.m_exprParser.parseConstantExpr();
            
            sItem.itemType.isArray = true;
            offset += (sItem.itemType.size / 8) * num;
            m_scanner.scan();
        }

        offset += sItem.itemType.size / 8;

        structType.contents.push_back(sItem);
        
        m_parser.match(Token::Tokens::SEMICOLON);

        if (m_scanner.token().token() == Token::Tokens::R_BRACE)
            break;
    }
    if (i == STRUCT_MAX_ITEMS)
        err.fatal("Too many items in struct (max 1023)");

noItems:;
    structType.size = offset;

    m_scanner.scan();

    m_typeList.replace(*structType.name, structType);
    
    return mkAstLeaf(AST::PADDING, 0, structType, 0, 0);
}

struct ast_node *StatementParser::declUnion(int sc)
{
    struct Type unionType;
    unionType.primType   = 0;
    unionType.ptrDepth   = 0;
    unionType.incomplete = false;
    unionType.typeType   = TypeTypes::UNION;
    unionType.isArray    = false;
    bool redecl          = false;
    string s = "anonymous" + to_string(anonStructCount++);
    
    if (m_scanner.token().token() == Token::Tokens::IDENTIFIER)
    {
        s = m_scanner.identifier();
        m_scanner.scan();
    }

    unionType.name = new string(s);

    if (s.compare("<anonymous>") &&
        m_typeList.getType(*unionType.name).typeType != 0)
    {
        redecl = true;

        if (!m_typeList.getType(*unionType.name).incomplete)
            err.fatal("trying to initialise an already initialized struct");
    }

    if (m_scanner.token().token() == Token::Tokens::SEMICOLON)
    {
        // Forward declare
        unionType.incomplete = true;

        // Double forward declare ??
        if (!redecl)
            m_typeList.addType(unionType);

        return mkAstLeaf(AST::Types::PADDING, 0, 0, 0);
    }
    
    m_parser.match(Token::Tokens::L_BRACE);

    struct StructItem sItem;
    int largestSize = 0;

    if (m_scanner.token().token() == Token::Token::R_BRACE)
        goto noItems;

    int i;
    for (i = 0; i < UNION_MAX_ITEMS; i++)
    {
        sItem.itemType = m_parser.parseType();
        if (sItem.itemType.typeType == 0)
            err.unknownType(&sItem.itemType);

        m_parser.match(Token::Tokens::IDENTIFIER);
        sItem.name = m_scanner.identifier();
        sItem.offset = 0;
        
        if (m_scanner.token().token() == Token::Tokens::L_BRACKET)
        {
            m_scanner.scan();
            int num = m_parser.m_exprParser.parseConstantExpr();
            
            if (sItem.itemType.size / 8 * num > largestSize)
                largestSize = sItem.itemType.size / 8 * num;
            sItem.itemType.isArray = true;
            m_scanner.scan();
        }

        if (sItem.itemType.size / 8 > largestSize)
            largestSize = sItem.itemType.size / 8;

        unionType.contents.push_back(sItem);
        
        m_parser.match(Token::Tokens::SEMICOLON);

        if (m_scanner.token().token() == Token::Tokens::R_BRACE)
            break;
    }
    if (i == STRUCT_MAX_ITEMS)
        err.fatal("Too many items in union (max 1023)");

noItems:;
    unionType.size = largestSize;

    m_scanner.scan();

    if (redecl)
        m_typeList.replace(*unionType.name, unionType);
    else
        m_typeList.addType(unionType);

    return mkAstLeaf(AST::PADDING, 0, unionType, 0, 0);
    
}
