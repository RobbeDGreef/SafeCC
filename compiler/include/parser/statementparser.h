#pragma once

#include <ast.h>
#include <core.h>
#include <generator.h>
#include <scanner.h>
#include <types.h>

// Forward declare parser to avoid include loops
class Parser;

class StatementParser
{

  private:
    Scanner   &m_scanner;
    Parser    &m_parser;
    Generator &m_generator;
    int m_lastOffset = 0;
  
  private:
    struct ast_node *parseBlock(int parentOp=0, bool newScope=true);
    struct ast_node *parseBlock(vector<struct Symbol> arguments);
    void             parseVariableArgParam(struct Symbol *s);

    struct ast_node *parseArrayInit(struct Type type, struct Symbol sym);
    struct ast_node *parseVarInit(struct Type type, struct Symbol sym);
    struct ast_node *parseStructInit(struct Type type, struct Symbol sym);

    struct ast_node *variableDecl(struct Type type, int storageClass);
    struct ast_node *variableAssignment(struct ast_node *lvalue);
    struct ast_node *functionDecl(struct Type type, int storageClass);
    struct ast_node *returnStatement();

    struct ast_node *ifStatement();
    struct ast_node *whileStatement();
    struct ast_node *forStatement();
    struct ast_node *doWhileStatement();
    
    struct ast_node *switchStatement();
    struct ast_node *switchCaseStatement();
    struct ast_node *switchDefaultStatement();
    struct ast_node *switchWalk(struct ast_node *tree);
    
    struct ast_node *comparison();
    struct ast_node *gotoStatement();
    struct ast_node *parseLabel(string label);

    struct ast_node *parseStatement(int parentTok=0);
    struct ast_node *parseDeclaration(struct Type t, int storageClass);
    struct ast_node *parseTypedef();
    struct ast_node *structInit(struct ast_node *tree, struct ast_node *ident,
                                struct ast_node *right, struct Symbol *sym,
                                int idx);

  public:
    struct ast_node *declStruct(int storageClass);
    struct ast_node *declUnion(int storageClass);
    struct ast_node *declEnum();
    struct ast_node *functionCall();
    struct ast_node *_parseBlock(int parentOp=0, struct ast_node *left=NULL);
    StatementParser(Scanner &scanner, Parser &parser, Generator &gen);
};
