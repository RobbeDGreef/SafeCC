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
    TypeList & m_typeList;

  private:
    struct ast_node *parsePrimary(struct Type *type);
    struct ast_node *parsePrefixOperator(struct Type *type);
    struct ast_node *parseBinaryOperator(int prev_prec, struct Type *type);
    struct ast_node *parseLeft(struct Type *t);
    struct ast_node *parseTypeCast(struct Type *t);
    int              getOperatorPrecedence(int token);
    struct ast_node *checkArithmetic(struct ast_node *l, struct ast_node *r,
                                     int tok);
    struct ast_node *parseParentheses(struct Type *ltype);
    struct ast_node *parseArrayAccess(struct ast_node *prim, bool access);
    struct ast_node *parseStructAccess(struct ast_node *prim, bool access);

  public:
    int              parseConstantExpr();
    struct ast_node *parsePostfixOperator(struct ast_node *tree, bool access);
    struct ast_node *parseBinaryOperation(int prev_prec, struct Type type);
    ExpressionParser(Scanner &scanner, Parser &parser, Generator &gen,
                     TypeList &typelist);
};