#include <parser/statementparser.h>
#include <parser/parser.h>
#include <core.h>
#include <config.h>
#include <errorhandler.h>
#include <symbols.h>

struct ast_node *StatementParser::declEnum()
{
    struct Type enumType = INTTYPE;
    struct Symbol enumItem;
    enumItem.varType = INTTYPE;
    enumItem.varType.typeType = TypeTypes::CONSTANT;
    bool redecl = false;
    int lastnum = 0;
    
    m_parser.match(Token::Tokens::IDENTIFIER);
    
    enumType.name = new string(m_scanner.identifier());
    enumType.typeType = TypeTypes::ENUM;
    
    if (m_scanner.token().token() == Token::Tokens::SEMICOLON)
    {
        err.pedanticWarning("ISO C forbids forward references to enum types");
        enumType.incomplete = true;
    }
    
    m_typeList.addType(enumType);
    
    if (m_typeList.getType(*enumType.name).typeType != 0)
        redecl = true;
    
    m_parser.match(Token::Tokens::L_BRACE);
    
    if (m_scanner.token().token() == Token::Tokens::ENUM)
        goto noItems;
    
    for (int i = 0; i < ENUM_MAX_ITEMS; i++)
    {
        m_parser.match(Token::Tokens::IDENTIFIER);
        enumItem.name = m_scanner.identifier();
        
        if (m_scanner.token().token() == Token::Tokens::EQUALSIGN)
        {
            m_scanner.scan();
            lastnum = m_parser.m_exprParser.parseConstantExpr();
        }
        
        enumItem.value = lastnum++;
        g_symtable.pushSymbol(enumItem);
        
        if (m_scanner.token().token() == Token::Tokens::R_BRACE)
            break;
        
        m_parser.match(Token::Tokens::COMMA);
    }

noItems:;
    m_scanner.scan();
    
    return mkAstLeaf(AST::Types::PADDING, 0, 0, 0);
}

int evaluateConstant(struct ast_node *tree)
{
    int leftreg = 0;
    int rightreg = 0;
    
    if (tree->left)
        leftreg = evaluateConstant(tree->left);
    
    if (tree->right)
        rightreg = evaluateConstant(tree->right);
        
    switch (tree->operation)
    {
    case AST::Types::ADD:
        return leftreg + rightreg;
    case AST::Types::SUBTRACT:
        return leftreg - rightreg;
    case AST::Types::MULTIPLY:
        return leftreg * rightreg;
    case AST::Types::DIVIDE:
        return leftreg / rightreg;
    case AST::Types::INTLIT:
        return tree->value;
    }
    
    err.fatal("unknown ast");
    return 0;
}

int ExpressionParser::parseConstantExpr()
{
    struct ast_node *opp = parseBinaryOperation(0, INTTYPE);
    int val = evaluateConstant(opp);
    return val;
}