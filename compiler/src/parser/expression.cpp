#include <errorhandler.h>
#include <parser/parser.h>
#include <symbols.h>
#include <types.h>

ExpressionParser::ExpressionParser(Scanner &scanner, Parser &parser,
                                   Generator &gen, TypeList &typelist)
    : m_scanner(scanner), m_parser(parser), m_generator(gen),
      m_typeList(typelist)
{
}

struct ast_node *ExpressionParser::parseSizeof()
{
    m_scanner.scan();

    struct ast_node *ptr  = parseLeft(&NULLTYPE);
    int size;
    
    if (ptr->operation == AST::Types::IDENTIFIER)
        size = getTypeSize(*g_symtable.getSymbol(ptr->value));
    else
        size = ptr->type.size;
    
    int              l    = m_scanner.curLine();
    int              c    = m_scanner.curChar();

    return mkAstLeaf(AST::Types::INTLIT, size, INTTYPE, l, c);
}

struct ast_node *ExpressionParser::parseParentheses(struct Type *ltype)
{
    m_parser.match(Token::Tokens::L_PAREN);
    
    struct ast_node *ret = parseTypeCast(ltype);

    if (!ret)
    {
        ret = parseBinaryOperator(0, ltype);
        m_parser.match(Token::Tokens::R_PAREN);
    }

    return ret;
}

struct ast_node *ExpressionParser::parseTypeCast(struct Type *ltype)
{
    int              tok = m_scanner.token().token();
    struct ast_node *ret;
    struct Type      type;
    switch (tok)
    {
    case Token::Tokens::SIGNED:
    case Token::Tokens::UNSIGNED:
    case Token::Tokens::VOID:
    case Token::Tokens::CHAR:
    case Token::Tokens::SHORT:
    case Token::Tokens::INT:
    case Token::Tokens::LONG:
    case Token::Tokens::STRUCT:
    case Token::Tokens::UNION:
    case Token::Tokens::IDENTIFIER:
        type = m_parser.parseType();
        if (type.typeType == 0)
            return NULL;

        m_parser.match(Token::Tokens::R_PAREN);
        
        ret       = parseBinaryOperation(0, NULLTYPE);
        ret->type = type;
        return ret;

    default:
        return NULL;
    }
}

bool closingStatement(int tok)
{
    if (tok == Token::Tokens::SEMICOLON || tok == Token::Tokens::R_PAREN ||
        tok == Token::Tokens::COMMA || tok == Token::Tokens::R_BRACE ||
        tok == Token::Tokens::R_BRACKET)
        return true;
    
    return false;
}

// Parses literal tokens to ast nodes and returns them
//
// Takes a type parameter to check the literal against but this can be a
// NULLTYPE
struct ast_node *ExpressionParser::parsePrimary(struct Type *ltype)
{
    struct ast_node *node;
    struct Symbol *  s;
    int              id;
    int              val;
    int tok = m_scanner.token().token();

    switch (tok)
    {
    case Token::Tokens::L_PAREN:
        node = parseParentheses(ltype);
        break;

    case Token::Tokens::INTLIT:
        val = m_scanner.token().intValue();

        if (ltype->typeType == 0)
            ltype = &(DEFAULTTYPE);

        if (!typeFits(ltype, val))
        {
            /* Truncate */
            err.warning("Value exceeds " + typeString(ltype) +
                        "'s bounds (value: " + to_string(val) + ")");

            val = truncateOverflow(*ltype, val);
            err.notice("Truncated value to '" + to_string(val) + "'");
        }
        
        if (ltype->ptrDepth)
            ltype = &(PTRTYPE);
        
        node = mkAstLeaf(AST::Types::INTLIT, val, *ltype, m_scanner.curLine(),
                         m_scanner.curChar());
        m_scanner.scan();

        break;

    case Token::Tokens::IDENTIFIER:
        id = g_symtable.findSymbol(m_scanner.identifier());
        if (id == -1)
            err.fatal("Unknown symbol: '" + m_scanner.identifier() + "'");

        s = g_symtable.getSymbol(id);
        
        if (s->varType.typeType == TypeTypes::CONSTANT)
        {
            node = mkAstLeaf(AST::Types::INTLIT, s->value, s->varType, m_scanner.curLine(),
                         m_scanner.curChar());
            m_scanner.scan();
            break;
        }

        if (ltype->typeType == 0)
        {
            ltype->primType = s->varType.primType;
            ltype->size     = s->varType.size;
            ltype->isSigned = s->varType.isSigned;
            ltype->ptrDepth = s->varType.ptrDepth;
        }

        m_scanner.scan();

        // Scan's can mess up the symtable (due to strings)
        s = g_symtable.getSymbol(id);

        if (m_scanner.token().token() == Token::Tokens::L_PAREN)
        {
            return m_parser.m_statementParser.functionCall();
        }

        node = mkAstLeaf(AST::Types::IDENTIFIER, id, s->varType,
                         m_scanner.curLine(), m_scanner.curChar());

        break;

    case Token::Tokens::STRINGLIT:
        val = m_scanner.token().intValue();
        s   = g_symtable.getSymbol(val);

        if (s == NULL)
            err.fatal("Could not find string literal in symbol table");

        node = mkAstLeaf(AST::Types::IDENTIFIER, val, s->varType,
                         m_scanner.curLine(), m_scanner.curChar());

        m_scanner.scan();
        break;
    
    case Token::Tokens::SIZEOF:
        node = parseSizeof();
        break;

    default:
        if (closingStatement(tok))
        {
            node = mkAstLeaf(AST::Types::PADDING, 0, m_scanner.curLine(),
                              m_scanner.curChar());
            break;
        }
        
        err.unexpectedToken(m_scanner.token().token());
    }

    return node;
}
// Parses prefix operators like * and &
//
// Takes one type parameter but this can be NULLTYPE
struct ast_node *ExpressionParser::parsePrefixOperator(struct Type *ltype)
{
    struct ast_node *node = NULL;
    int              id;
    struct Type      type;
    struct ast_node *left = NULL;
    struct ast_node *tmp = NULL;
    
    switch (m_scanner.token().token())
    {
    case Token::Tokens::AMPERSANT:
        m_scanner.scan();

        node = parsePostfixOperator(parsePrimary(ltype), false);

        if (node->operation != AST::Types::IDENTIFIER && 
            (node->operation != AST::Types::ADD && 
            node->type.typeType == TypeTypes::STRUCT))
            err.fatal("Lvalue required as argument to unary '&'");

        if (node->operation == AST::Types::ADD)
            left = node;
        
        else if ((id = g_symtable.findSymbol(m_scanner.identifier())) == -1)
            err.fatal("Invalid operand to unary '&'");


        type = node->type;
        type.ptrDepth++;
        type.size = PTR_SIZE;
        
        node = mkAstUnary(AST::Types::LOADLOCATION, left, id, type,
                             m_scanner.curLine(), m_scanner.curChar());

        break;

    case Token::Tokens::STAR:
        m_scanner.scan();

        node = parseLeft(ltype);

        if (!node->type.ptrDepth)
            err.fatal("Unsupported type to unary: " +
                      typeString((&node->type)) + " (expected pointer type)");

        type = node->type;
        dereference(&type);

        if (type.typeType == TypeTypes::STRUCT && !type.ptrDepth)
            node->type = type;
        else
            node = mkAstUnary(AST::Types::PTRACCESS, node, 0, type, node->line,
                          node->c);

        break;

    case Token::Tokens::MINUS:
        m_scanner.scan();
        node = parseLeft(ltype);

        if ((!node->type.isSigned) || node->type.ptrDepth)
            err.fatal("Cannot negate type " + HL(typeString((&node->type))));

        node = mkAstUnary(AST::Types::NEGATE, node, 0, node->type, node->line,
                          node->c);

        break;

    case Token::Tokens::TIDDLE:
        m_scanner.scan();
        
        node = parseLeft(ltype);
        
        if (node->type.ptrDepth || node->type.typeType != TypeTypes::VARIABLE)
            err.fatal("Cannot preform bitwise not on " + HL(typeString(&node->type)));
        
        node = mkAstUnary(AST::Types::NOT, node, 0, node->type, node->line, 
                          node->c);
        
        break;
        
    case Token::Tokens::LOGNOT:
        m_scanner.scan();
        node = parseLeft(ltype);
        node = mkAstUnary(AST::Types::LOGNOT, node, 0, node->type, node->line, node->c);
        
        break;
    
    case Token::Tokens::INC:
        m_scanner.scan();
        node = parseLeft(ltype);
        
        if (node->operation != AST::Types::IDENTIFIER)
            err.fatal("Cannot increment a non-lvalue object");
        
        left = mkAstLeaf(AST::Types::INTLIT, 1, INTTYPE, node->line, node->c);
        
        tmp = checkArithmetic(node, left, Token::Tokens::PLUS);
        if (tmp)
        {
            if (node->type.ptrDepth)
                left = tmp;
        }
        
        // If value is 1 it means increment after value return
        // if it is 0 it means increment before value return
        node = mkAstNode(AST::Types::INCREMENT, node, NULL, left, 0, node->type,
                         node->line, node->c);
    
        break;
    
    case Token::Tokens::DEC:
        m_scanner.scan();
        node = parseLeft(ltype);
        
        if (node->operation != AST::Types::IDENTIFIER)
            err.fatal("Cannot decrement a non-lvalue object");
        
        left = mkAstLeaf(AST::Types::INTLIT, 1, INTTYPE, node->line, node->c);
        
        tmp = checkArithmetic(node, left, Token::Tokens::MINUS);
        if (tmp)
        {
            if (node->type.ptrDepth)
                left = tmp;
        }
        
        // If value is 1 it means increment after value return
        // if it is 0 it means increment before value return
        node = mkAstNode(AST::Types::DECREMENT, node, NULL, left, 0, node->type,
                         node->line, node->c);
    
        break;
    
    default:
        node = parsePrimary(ltype);
    }

    return node;
}

// Parses array accesses like x[0] and returns the ast tree
//
// Takes two parametes, the previously parsed ast_node (the x in the expression)
// and a bool to specify whether a ptraccess should be generated in the returned
// tree
struct ast_node *ExpressionParser::parseArrayAccess(struct ast_node *primary,
                                                    bool             access)
{
    if (m_scanner.token().token() != Token::Tokens::L_BRACKET)
        return primary;
    
    m_scanner.scan();
    
    struct ast_node *right;
    struct ast_node *idx;
    struct Type      type;

    if (!primary->type.ptrDepth)
    {
        err.fatal("Unsupported type to array accessor " +
                  typeString((&primary->type)) + " expected pointer type");
    }

    idx = parseBinaryOperation(0, DEFAULTTYPE);
    
    //idx = mkAstUnary(AST::Types::NEGATE, idx, idx->value, idx->type,
    //                 idx->line, idx->c);

    m_parser.match(Token::Tokens::R_BRACKET);

    type = primary->type;
    dereference(&type);

    right = mkAstLeaf(AST::Types::INTLIT, type.size, idx->type,
                      m_scanner.curLine(), m_scanner.curChar());

    idx = mkAstNode(AST::Types::MULTIPLY, idx, NULL, right, 0, primary->type,
                    m_scanner.curLine(), m_scanner.curChar());

    primary = mkAstNode(AST::Types::ADD, primary, NULL, idx, 0, primary->type,
                        m_scanner.curLine(), m_scanner.curChar());

    if (access)
        primary = mkAstUnary(AST::Types::PTRACCESS, primary, 0, type,
                             primary->line, primary->c);

    primary = parseArrayAccess(primary, access);

    return primary;
}

struct ast_node *ExpressionParser::parseStructAccess(struct ast_node *prim, bool access)
{
    bool ptraccess = false;
    
    if (m_scanner.token().token() == Token::Tokens::MINUS)
    {
        m_scanner.scan();
        if (m_scanner.token().token() != Token::Tokens::GREATERTHAN)
        {
            m_scanner.putbackToken(m_scanner.token());
            m_scanner.token().set(Token::Tokens::MINUS, m_scanner.curLine(),
                                  m_scanner.curChar());
            return prim;
        }
        ptraccess = true;
    }
    
    if (prim->type.typeType != TypeTypes::STRUCT)
        err.fatal("cannot preform struct access on non-aggregate type");

    if (ptraccess && !prim->type.ptrDepth)
        err.incorrectAccessor(ptraccess);
    if (!ptraccess && prim->type.ptrDepth)
        err.incorrectAccessor(ptraccess);
    
    m_scanner.scan();
    if (m_scanner.token().token() != Token::Tokens::IDENTIFIER)
        err.expectedToken(Token::Tokens::IDENTIFIER);
    
    struct Type t = prim->type;
    int idx = findStructItem(m_scanner.identifier(), t);
    int offset = t.contents[idx].offset;
    int size = t.contents[idx].itemType.size;
    t = t.contents[idx].itemType;
    
    int l = m_scanner.curLine();
    int c = m_scanner.curChar();
    struct ast_node *offsetNode = mkAstLeaf(AST::INTLIT, offset, INTTYPE, l, c);
    prim = mkAstNode(AST::Types::ADD, prim, NULL, offsetNode, 0, t, l, c);
     
    if (access)
        prim = mkAstUnary(AST::Types::PTRACCESS, prim, size, t, l, c);
    
    m_scanner.scan();
    
    return prim;
}

// Checks if the two nodes are pointer and if they are checks the arithmetic
// between them.
//
// Takes left and right ast nodes to check and a token that holds the arithmetic
// instruction
struct ast_node *ExpressionParser::checkArithmetic(struct ast_node *left,
                                                   struct ast_node *right,
                                                   int              tok)
{
    if (tok == Token::Tokens::EQUALSIGN)
        return NULL;
    
    struct ast_node *ret = NULL;

    if ((left->type.typeType == TypeTypes::STRUCT && !left->type.ptrDepth) ||
        (right->type.typeType == TypeTypes::STRUCT && !right->type.ptrDepth))
        err.fatal("Cannot preform binary arithmetic on structs");

    if (left->type.ptrDepth || right->type.ptrDepth)
    {
        if (tok != Token::Tokens::MINUS && tok != Token::Tokens::PLUS)
        {
            err.fatal("Pointer arithmetic only allows + and - to be used ");
        }
        else if (left->type.ptrDepth && right->type.ptrDepth &&
                 tok == Token::Tokens::PLUS)
        {
            err.fatal("Pointer arithmetic only allows - to be used between "
                      "pointers");
        }

        if (!(left->type.ptrDepth && right->type.ptrDepth))
        {
            // Scaling

            struct ast_node *ptr  = left->type.ptrDepth ? left : right;
            struct ast_node *nptr = left->type.ptrDepth ? right : left;
            struct Type      t;
            
            t.isArray           = false;
            t.isSigned          = false;
            t.ptrDepth          = 0;
            t.primType          = PrimitiveTypes::INT;
            t.size              = PTR_SIZE;
            nptr->type.size     = PTR_SIZE;
            nptr->type.primType = PrimitiveTypes::INT;

            ret = mkAstLeaf(AST::Types::INTLIT, ptr->type.size, t,
                            left->line, left->c);
            return mkAstNode(AST::Types::MULTIPLY, ret, NULL, nptr, 0,
                             left->line, left->c);
        }
    }
    return ret;
}

struct ast_node *ExpressionParser::parsePostfixOperator(struct ast_node *tree, bool access)
{
    struct ast_node *left = NULL;
    struct ast_node *tmp = NULL;
    
    switch (m_scanner.token().token())
    {
    case Token::Tokens::L_BRACKET:
        return parseArrayAccess(tree, access);
    
    case Token::Tokens::MINUS:
    case Token::Tokens::DOT:
        return parseStructAccess(tree, access);
    
    case Token::Tokens::INC:
        m_scanner.scan();
        
        if (tree->operation != AST::Types::IDENTIFIER)
            err.fatal("Cannot increment a non-lvalue object");
        
        left = mkAstLeaf(AST::Types::INTLIT, 1, INTTYPE, tree->line, tree->c);
        
        tmp = checkArithmetic(tree, left, Token::Tokens::PLUS);
        if (tmp)
        {
            if (tree->type.ptrDepth)
                left = tmp;
        }
        
        // If value is 1 it means increment after value return
        // if it is 0 it means increment before value return
        tree = mkAstNode(AST::Types::INCREMENT, tree, NULL, left, 1, tree->type,
                         tree->line, tree->c);
    
        return tree;
    
    case Token::Tokens::DEC:
        m_scanner.scan();
        
        if (tree->operation != AST::Types::IDENTIFIER)
            err.fatal("Cannot decrement a non-lvalue object");
        
        left = mkAstLeaf(AST::Types::INTLIT, 1, INTTYPE, tree->line, tree->c);
        
        tmp = checkArithmetic(tree, left, Token::Tokens::MINUS);
        if (tmp)
        {
            if (tree->type.ptrDepth)
                left = tmp;
        }
        
        // If value is 1 it means increment after value return
        // if it is 0 it means increment before value return
        tree = mkAstNode(AST::Types::DECREMENT, tree, NULL, left, 1, tree->type,
                         tree->line, tree->c);
    
        return tree;
    
    default:
        return tree;
    }
}

// Simply parses one side of a binary operation
//
// Takes type parameter to parse the operators with
struct ast_node *ExpressionParser::parseLeft(struct Type *type)
{
    struct ast_node *left = parsePrefixOperator(type);
    return parsePostfixOperator(left, true);
}

// Recursive parsing function for binary operations. Returns the roots
// of the AST
//
// Takes the previous operator precedence and a type parameter to typecheck the
// previous operation against
struct ast_node *ExpressionParser::parseBinaryOperator(int          prev_prec,
                                                       struct Type *type)
{
    struct ast_node *left;
    
    left = parseLeft(type);

    int tok = m_scanner.token().token();

    if (closingStatement(tok))
        return left;
        
    if (tok == AST::Types::ASSIGN)
        type = &left->type;

    while (getOperatorPrecedence(tok) > prev_prec)
    {
        m_scanner.scan();

        struct ast_node *right = parseBinaryOperator(OperatorPrecedence[tok],
                                                     type);

        struct ast_node *tmp = checkArithmetic(left, right, tok);
        if (tmp)
        {
            if (left->type.ptrDepth)
                right = tmp;
            else
                left = tmp;

            tmp = NULL;
        }
        else
        {
            bool onlyright = false;
            if (tok == AST::Types::ASSIGN)
                onlyright = true;
            
            // Will automatically widen registers if needed
            tmp = typeCompatible(left, right, onlyright);
            if (tmp)
            {
                if (tmp->left == left)
                    left = tmp;
                else
                    right = tmp;
            }
        }

        // Join the trees
        left = mkAstNode(tokenToAst(tok, m_scanner), left, NULL, right, 0,
                         left->type, m_scanner.curLine(), m_scanner.curChar());

        tok = m_scanner.token().token();

        if (closingStatement(tok))
            return left;
    }

    return left;
}

// The parse binary operation frontend. Will call parseBinaryOperator and
// typecheck its result
//
// takes the previous precedence and a type for parseBinaryOperator
struct ast_node *ExpressionParser::parseBinaryOperation(int         prev_prec,
                                                        struct Type type)
{
    struct Type inttypes = type;

    if (type.typeType != 0 && type.typeType != TypeTypes::STRUCT)
    {
        if (type.ptrDepth)
        {
            inttypes.size     = PTR_SIZE;
            inttypes.primType = INT;
            inttypes.ptrDepth = 0;
        }
    }

    struct ast_node *node = parseBinaryOperator(prev_prec, &inttypes);
    struct ast_node *tmp;
    
    if (type.typeType != 0)
    {
        // This is just a padding instruction to hold the type specifier and
        // test it agains the actual binary operation
        struct ast_node *pad = mkAstLeaf(AST::Types::PADDING, 0, type,
                                         m_scanner.curLine(),
                                         m_scanner.curChar());

        tmp = typeCompatible(pad, node, true);
        if (tmp)
            node = tmp;
        delete pad;
    }

    return node;
}

// Returns the operator precedence or throws error if the token is not in the
// list
//
// Takes the operator token as a parameterk
int ExpressionParser::getOperatorPrecedence(int token)
{
    int prec = OperatorPrecedence[token];
    if (prec == 0)
        err.unexpectedToken(token);

    return prec;
}
