#pragma once

#include <attributes.h>
#include <core.h>
#include <scanner.h>
#include <ast.h>
#include <generator.h>
#include <parser/expressionparser.h>
#include <parser/statementparser.h>

static const int OperatorPrecedence[] = {
    0,          // EOF
    12, 12,     // + -
    13, 13, 13, // * / %
    6, 7, 8,    // | ^ &
    11, 11,     // << >>
    9, 9,       // == !=
    10, 10, 10, 10, // < > <= >=
    5, 4,       // && ||
    0,
    1,          // =
    3, 2        // : ?  (note that ':' in c isn't really an operator but 
                //       our parser will correctly parse the ternary this way)
};

class Parser
{

private:
    Scanner             &m_scanner;
    Generator           &m_generator;
    StatementParser     m_statementParser;
    ExpressionParser    m_exprParser;

private:
    void match(int t);
    void match(int t1, int t2);
    void match(int t1, int t2, string error);
    void matchNoScan(int t);
    Type parseType();
    ast_node *_declAggregateType(int tok, string s = "");
    vector<Attribute> parseAttr();
    vector<Attribute> _parseAttr();
    vector<Attribute> _parseParen(vector <Attribute> attributes);

public:
    Parser(Scanner &scanner, Generator &generator);
    ast_node *parserMain();

    friend class StatementParser;
    friend class ExpressionParser;
};