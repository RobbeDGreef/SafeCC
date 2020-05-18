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
    ast_node *parseBlock(int parentOp=0, bool newScope=true);
    ast_node *parseBlock(vector<Symbol> arguments);
    void             parseVariableArgParam(Symbol *s);

    ast_node *parseArrayInit(Type type, Symbol sym);
    ast_node *parseVarInit(Type type, Symbol sym);
    ast_node *parseStructInit(Type type, Symbol sym);

    ast_node *variableDecl(Type type, int storageClass);
    ast_node *variableAssignment(ast_node *lvalue);
    ast_node *functionDecl(Type type, int storageClass);
    ast_node *returnStatement();
    void parseFuncAttrs(Type *t);

    ast_node *ifStatement();
    ast_node *whileStatement();
    ast_node *forStatement();
    ast_node *doWhileStatement();
    
    ast_node *switchStatement();
    ast_node *switchCaseStatement();
    ast_node *switchDefaultStatement();
    ast_node *switchWalk(ast_node *tree);
    
    ast_node *comparison();
    ast_node *gotoStatement();
    ast_node *parseLabel(string label);

    ast_node *parseStatement(int parentTok=0);
    ast_node *parseDeclaration(Type t, int storageClass);
    ast_node *parseTypedef();
    ast_node *structInit(ast_node *tree, ast_node *ident,
                                ast_node *right, Symbol *sym,
                                int idx);

  public:
    ast_node *declStruct(int storageClass, string s = "");
    ast_node *declUnion(int storageClass, string s = "");
    ast_node *declEnum(string s = "");
    ast_node *functionCall();
    ast_node *_parseBlock(int parentOp=0, ast_node *left=NULL);
    StatementParser(Scanner &scanner, Parser &parser, Generator &gen);
};
