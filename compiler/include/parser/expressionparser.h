#pragma once

#include <ast.h>
#include <core.h>
#include <scanner.h>
#include <types.h>

// Forward declare parser to avoid include loops
class Parser;

class ExpressionParser
{
  private:
    Scanner &  m_scanner;
    Parser &   m_parser;
    Generator &m_generator;

  private:
    ast_node *parseComplexAssign(ast_node *left, int tok);
    ast_node *parsePrimary(Type *type);
    ast_node *parsePrefixOperator(Type *type);
    ast_node *parseBinaryOperator(int prevPrec, Type *type,
                                         int prevTok = -1);
    ast_node *parseLeft(Type *t);
    ast_node *parseTypeCast(Type *t);
    int              getOperatorPrecedence(int token);
    ast_node *checkArithmetic(ast_node *l, ast_node *r,
                                     int tok);
    ast_node *parseParentheses(Type *ltype);
    ast_node *parseArrayAccess(ast_node *prim, bool access);
    ast_node *parseStructAccess(ast_node *prim, bool access);
    ast_node *parseTernaryCondition(Type *type);

  public:
    ast_node *parseSizeof();
    int              parseConstantExpr();
    ast_node *parsePostfixOperator(ast_node *tree, bool access);
    ast_node *parseBinaryOperation(int prev_prec, Type type);
    ExpressionParser(Scanner &scanner, Parser &parser, Generator &gen);
};