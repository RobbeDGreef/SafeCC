#pragma once

#include <core.h>
#include <scanner.h>
#include <ast.h>
#include <generator.h>
#include <parser/expressionparser.h>
#include <parser/statementparser.h>

static const int OperatorPrecedence[] = {
    0,          // EOF
    10, 10,     // + -
    11, 11, 11, // * / %
    4, 5, 6,    // | ^ &
    9, 9,       // << >>
    7, 7,       // == !=
    8, 8, 8, 8, // < > <= >=
    3, 2,       // && ||
    0,
    1           // =
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