#include <core.h>
#include <errorhandler.h>
#include <parser/parser.h>
#include <symbols.h>
#include <token.h>

vector<Type> extractTypes(vector<Symbol> arguments)
{
    vector<Type> types;
    for (Symbol s : arguments)
    {
        types.push_back(s.varType);
    }
    return types;
}

ast_node *StatementParser::returnStatement()
{
    Symbol *fsym = g_symtable.getSymbol(g_symtable.currentFuncIdx());

    m_scanner.scan();
    ast_node *tree = m_parser.m_exprParser.parseBinaryOperation(0,
                                                                       fsym->varType);
    
    if (!tree)
        err.unknownSymbol(m_scanner.identifier());
    
    if (tree->type.memSpot)
    {
        if (!fsym->varType.memSpot)
            fsym->varType.memSpot = new MemorySpot(tree->type.memSpot, true);
        
        fsym->varType.memSpot->setName("the return value of " + fsym->name);
    }
    
    return mkAstUnary(AST::Types::RETURN, tree, g_symtable.currentFuncIdx(),
                      m_scanner.curLine(), m_scanner.curChar());
}

void StatementParser::parseVariableArgParam(Symbol *argsym)
{
    m_parser.match(Token::Tokens::DOT);
    m_parser.match(Token::Tokens::DOT);
    m_parser.match(Token::Tokens::DOT);

    if (m_scanner.token().token() != Token::Tokens::R_PAREN)
        err.expectedToken(Token::Tokens::R_PAREN);

    argsym->variableArg = true;
}

ast_node *StatementParser::functionDecl(Type type, int sc)
{
    int presetFunc = true;
    int nameIdx    = g_symtable.findSymbol(m_scanner.identifier());

    if (nameIdx == -1)
    {
        nameIdx    = g_symtable.addSymbol(m_scanner.identifier(), 0,
                                       SymbolTable::SymTypes::FUNCTION, type, sc);
        presetFunc = false;
    }

    // Functions can only be declared in global scope
    if (!g_symtable.isCurrentScopeGlobal())
        err.fatal("Function '" + g_symtable.getSymbol(nameIdx)->name +
                  "' is not "
                  "declared in global scope\nC doesn't allow nested "
                  "functions!");

    // Changes every time we parse a new function
    g_symtable.changeCurFunc(nameIdx);

    m_parser.match(Token::Tokens::L_PAREN);

    ast_node *     arg;
    Type           argtype;
    vector<Symbol> arguments;
    Symbol         argsym;
    Symbol *       function;
    argsym.symType = SymbolTable::SymTypes::ARGUMENT;
    argsym.value   = 0;

    if (m_scanner.token().token() == Token::Tokens::R_PAREN)
        goto noargs;

    for (int i = 0; i < MAX_ARGUMENTS_TO_FUNCTION; i++)
    {
        argsym.stackLoc = i;
        if (m_scanner.token().token() == Token::Tokens::DOT)
        {
            parseVariableArgParam(g_symtable.getSymbol(nameIdx));
            break;
        }

        argtype = m_parser.parseType();
        if (argtype.typeType == 0)
            err.unknownType(m_scanner.identifier());

        if (presetFunc && g_symtable.getSymbol(nameIdx)->arguments.size() == i ||
            presetFunc && !equalType(argtype,
                                     g_symtable.getSymbol(nameIdx)->arguments[i]))
        {
            err.fatal("Function was previously declared with different "
                      "argument types");
        }

        argsym.varType = argtype;

        if (m_scanner.token().token() == Token::Tokens::IDENTIFIER)
        {
            argsym.name = m_scanner.identifier();
            m_scanner.scan();
            
            if (m_scanner.token().token() == Token::Tokens::L_BRACKET)
            {
                m_scanner.scan();
                int amount = m_parser.m_exprParser.parseConstantExpr();
                m_parser.match(Token::Tokens::R_BRACKET);
                
                argsym.varType.isArray = true;
                argsym.varType.size *= amount;
                argsym.value = amount;
            }
        }

        if (m_scanner.token().token() == Token::Tokens::R_PAREN)
        {
            /* leave */
            arguments.push_back(argsym);
            break;
        }

        m_parser.match(Token::Tokens::COMMA);
        arguments.push_back(argsym);
    }
    /**
     *  Now there are two possibilities:
     *  - a new compound statement follows and thus we find a {
     *  - or this was a forward decleration and thus a ; follows
     *  either way the function must be added to the global sym table
     */

noargs:;

    function = g_symtable.getSymbol(nameIdx);
    if (presetFunc && function->arguments.size() != arguments.size())
        err.fatal("Function was previously declared with different argument "
                  "types");

    m_scanner.scan();

    if (!presetFunc)
    {
        function            = g_symtable.getSymbol(nameIdx);
        function->arguments = extractTypes(arguments);
    }

    if (m_scanner.token().token() == Token::Tokens::ATTRIBUTE)
        function->attributes = m_parser.parseAttr();

    if (m_scanner.token().token() == Token::Tokens::ASM)
        m_scanner.scanUntil(Token::Tokens::SEMICOLON);

    if (m_scanner.token().token() == Token::Tokens::SEMICOLON)
    {
        return mkAstLeaf(AST::Types::PADDING, 0, m_scanner.curLine(),
                         m_scanner.curChar());
    }

    if (type.typeType == TypeTypes::STRUCT && !type.ptrDepth && type.size > 8)
    {
        argsym.symType = SymbolTable::SymTypes::IMPLICIT;
        arguments.insert(arguments.begin(), argsym);
    }
    
    function->defined = true;
    ast_node *body = parseBlock(arguments);
    ErrorInfo errInfo = err.createErrorInfo();
    m_parser.match(Token::Tokens::R_BRACE);

    /* Generate the machine code */
    m_generator.generateFromAst(mkAstUnary(AST::Types::FUNCTION, body, nameIdx,
                                           m_scanner.curLine(),
                                           m_scanner.curChar()),
                                -1, 0);
    
    err.loadErrorInfo(errInfo);
    g_symtable.popScope(true, true);
    return NULL;
}

/// @brief  Generates a ast tree for a functioncall
ast_node *StatementParser::functionCall()
{
    int              id;
    Symbol *s;   
    ast_node *tree = NULL;
    ast_node *arg  = NULL;
    ast_node *tmp  = NULL;
    ast_node *off  = NULL;
    Type      type;
    int              i = -1;
    vector<int> removeMem;

    if ((id = g_symtable.findSymbol(m_scanner.identifier())) == -1)
    {
        err.fatal("Function '" + m_scanner.identifier() +
                  "' has not yet been declared");
    }
    
    s = g_symtable.getSymbol(id);
    Type returnType = s->varType;
    s->used = true;
    if (returnType.ptrDepth)
    {
        returnType.memSpot = new MemorySpot(s->varType.memSpot, true);
    }
    
    if (s->attributes.size())
    {
        if (hasAttr(s->attributes, Attributes::RETURNS_HEAPVAL))
        {
            MemorySpot *ms = new MemorySpot();
            ms->goodValues();
            ms->setMemLoc(MemorySpot::MemLoc::HEAP);
            returnType.memSpot->addReferencingTo(ms, 0);
        }
        
        if (hasAttr(s->attributes, Attributes::CLEARS_HEAPVAL))
        {
            removeMem = getAttr(s->attributes, Attributes::CLEARS_HEAPVAL).values;
        }
    }

    m_parser.match(Token::Tokens::L_PAREN);

    if (m_scanner.token().token() == Token::Token::R_PAREN)
        goto noarg;

    for (i = 0; i < g_symtable.getSymbol(id)->arguments.size(); i++)
    {
        arg = m_parser.m_exprParser.parseBinaryOperation(0, g_symtable.getSymbol(id)->arguments[i]);
        if (!arg)
            err.unknownSymbol(m_scanner.identifier());
        
        if (count(removeMem.begin(), removeMem.end(), i))
        {
            if (arg->type.memSpot && arg->type.memSpot->references() != -1)
            {
                int id = arg->type.memSpot->references();
                g_memTable.findMemorySpot(id)->destroy("");
            }
            else if (arg->type.memSpot)
            {
                arg->type.memSpot->setAccessGarbage(true);
            }
            else
            {
                err.memWarn("Trying to remove non-heap allocated object from heap?");
            }
        }
#if 0
        if (arg->type.typeType == TypeTypes::STRUCT && !arg->type.ptrDepth)
        {
            for (int j = 0; j < arg->type.contents.size(); j++)
            {
                int l      = m_scanner.curLine();
                int c      = m_scanner.curChar();
                int offset = arg->type.contents[j].offset;
                type       = arg->type.contents[j].itemType;
                off = mkAstLeaf(AST::Types::INTLIT, offset, INTTYPE, l, c);
                tmp = mkAstNode(AST::Types::ADD, arg, NULL, off, 0, arg->type,
                                l, c);
                tmp = mkAstUnary(AST::Types::PTRACCESS, tmp, type.size, type, l,
                                 c);
                tree = mkAstNode(AST::Types::FUNCTIONARGUMENT, tree, NULL, tmp,
                                 i, l, c);
            }
        }
        else
#endif
        tree = mkAstNode(AST::Types::FUNCTIONARGUMENT, tree, NULL, arg, i,
                         m_scanner.curLine(), m_scanner.curChar());

        if (m_scanner.token().token() == Token::Tokens::R_PAREN)
        {
            if (i < g_symtable.getSymbol(id)->arguments.size() - 1)
                err.fatal("Expected more parameters to function '" +
                          g_symtable.getSymbol(id)->name + "'");
            break;
        }

        m_parser.match(Token::Tokens::COMMA);
    }

    if (m_scanner.token().token() != Token::Tokens::R_PAREN &&
        g_symtable.getSymbol(id)->variableArg)
    {
        for (; i < MAX_ARGUMENTS_TO_FUNCTION; i++)
        {
            arg = m_parser.m_exprParser.parseBinaryOperation(0, NULLTYPE);
            if (!arg)
                err.unknownSymbol(m_scanner.identifier());
            if (arg->type.typeType == TypeTypes::STRUCT && !arg->type.ptrDepth)
            {
                for (int j = 0; j < arg->type.contents.size(); j++)
                {
                    int l      = m_scanner.curLine();
                    int c      = m_scanner.curChar();
                    int offset = arg->type.contents[j].offset;
                    type       = arg->type.contents[j].itemType;
                    off  = mkAstLeaf(AST::Types::INTLIT, offset, INTTYPE, l, c);
                    tmp  = mkAstNode(AST::Types::ADD, arg, NULL, off, 0,
                                    arg->type, l, c);
                    tmp  = mkAstUnary(AST::Types::PTRACCESS, tmp, type.size,
                                     type, l, c);
                    tree = mkAstNode(AST::Types::FUNCTIONARGUMENT, tree, NULL,
                                     tmp, i, l, c);
                }
            }
            else
                tree = mkAstNode(AST::Types::FUNCTIONARGUMENT, tree, NULL, arg,
                                 i, m_scanner.curLine(), m_scanner.curChar());

            if (m_scanner.token().token() == Token::Tokens::R_PAREN)
                break;

            m_parser.match(Token::Tokens::COMMA);
        }
    }
    else if (m_scanner.token().token() != Token::Tokens::R_PAREN)
        err.fatal("Too many arguments to function '" +
                  g_symtable.getSymbol(id)->name + "'");

noarg:;

    if (i < (int)g_symtable.getSymbol(id)->arguments.size() - 1)
        err.fatal("Expected more paramters to funcion '" +
                  g_symtable.getSymbol(id)->name + "'");

    m_scanner.scan();
    return mkAstUnary(AST::Types::FUNCTIONCALL, tree, id, returnType,
                      m_scanner.curLine(), m_scanner.curChar());
}