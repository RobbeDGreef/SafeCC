#pragma once

#include <core.h>
#include <scanner.h>
struct Type;

#define ESCAPE_RED     "\u001b[31;1m"
#define ESCAPE_PURPLE  "\u001b[35;1m"
#define ESCAPE_BLUE    "\u001b[36;1m"
#define ESCAPE_GREEN   "\u001b[33;1m"
#define ESCAPE_END     "\u001b[0m"
#define WARNING_PREFIX ESCAPE_PURPLE "warning " ESCAPE_END
#define FATAL_PREFIX   ESCAPE_RED "error " ESCAPE_END
#define NOTICE_PREFIX  ESCAPE_BLUE "notice " ESCAPE_END
#define TIP_PREFIX     ESCAPE_GREEN "tip " ESCAPE_END
#define HL_c           "\u001b[37;1m"
#define HL_c_end       "\u001b[0m"
#define HL(x)          string(HL_c) + (x) + string(HL_c_end)

struct ErrorInfo
{
    Token tok;
    string line;
    string func;
    string file;
    int lineNum;
    int charNum;
};

class ErrorHandler
{
  private:
    Scanner *m_scanner;
    ErrorInfo m_errInfo;
    int m_justLoadedInfo = false;

  public:
    /* flags can be enabled publicly */
    int  f_warningAsError    = false;
    int  f_conversionWarn    = false;
    bool f_ptrConversionWarn = true;
    int f_noMemChecking = false;

  private:
    void write(string str);

  public:
    ErrorHandler();
    void setupLinehandler(Scanner &scan);
    void loadErrorInfo(ErrorInfo ei);
    void unloadErrorInfo();
    void loadedErrorInfo();
    ErrorInfo getLoadedErrorInfo();
    ErrorInfo createErrorInfo();
    ErrorInfo createErrorInfo(int line, int c);

    void fatal(string str);
    void fatal(string str, int line, int c);
    void fatalNL(string str);
    void warning(string str);
    void warningNL(string str);
    void notice(string str);
    void noticeNL(string str);
    void lineError(ErrorInfo errInfo, string hl_color);
    void lineError(string color);
    void lineError(int l, int c, string color);

    void syntaxError(string str);
    void expectedType(Type *l, Type *r);
    void unexpectedToken(int token);
    void expectedToken(int token);
    void expectedToken(int token, int got);
    void expectedToken(int token, int token2, int got);
    void unexpectedParamToFunc(string func, Type *t);

    void conversionError(Type *l, Type *r);
    void conversionWarning(Type *l, Type *r);
    void ptrConversionWarning(Type *l, Type *r);

    void typeConversionError(Type *l, Type *r);
    void unknownType(Type *l);
    void unknownType(string s);
    void unknownSymbol(string s);
    void tip(string s);
    void tipNL(string s);
    void unknownStructItem(string s, Type t);
    void incorrectAccessor(bool ptr);
    void pedanticWarning(string warn);
    
    void memWarn(string s);
    void memNotice(string s);
    
};

extern ErrorHandler err;