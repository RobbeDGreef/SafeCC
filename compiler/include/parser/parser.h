#pragma once

#include <core.h>
#include <scanner.h>
#include <ast.h>
#include <generator.h>
#include <parser/expressionparser.h>
#include <parser/statementparser.h>

static const int OperatorPrecedence[] = {
     0,          // EOF
     9, 9,       // + -
     10, 10, 10, // * / %
     3, 4, 5,    // | ^ &
     8, 8,       // << >>
     6, 6,       // == !=
     7, 7, 7, 7, // < > <= >=
     2, 1        // && ||
};

class Parser
{

private:
    Scanner             &m_scanner;
    Generator           &m_generator;
    TypeList            m_typeList;
    StatementParser     m_statementParser;
    ExpressionParser    m_exprParser;

private:
    void match(int t);
    void match(int t1, int t2);
    void match(int t1, int t2, string error);
    void matchNoScan(int t);
    struct Type parseType();

public:
    Parser(Scanner &scanner, Generator &generator);
    struct ast_node *parserMain();

    friend class StatementParser;
    friend class ExpressionParser;
};