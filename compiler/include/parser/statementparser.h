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
    Scanner &  m_scanner;
    Parser &   m_parser;
    Generator &m_generator;
    TypeList & m_typeList;

  private:
    struct ast_node *parseBlock();
    struct ast_node *parseBlock(vector<struct Symbol> arguments);
    void             parseVariableArgParam(struct Symbol *s);

    struct ast_node *parseArrayInit(struct Type type, struct Symbol sym);
    struct ast_node *parseVarInit(struct Type type, struct Symbol sym);
    struct ast_node *parseStructInit(struct Type type, struct Symbol sym);

    struct ast_node *variableDecl(struct Type type, int storageClass);
    struct ast_node *variableAssignment(int ptrDepth);
    struct ast_node *functionDecl(struct Type type, int storageClass);
    struct ast_node *returnStatement();

    struct ast_node *ifStatement();
    struct ast_node *whileStatement();
    struct ast_node *forStatement();


    struct ast_node *parseStatement();
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
    struct ast_node *_parseBlock();
    StatementParser(Scanner &scanner, Parser &parser, Generator &gen,
                    TypeList &typelist);
};