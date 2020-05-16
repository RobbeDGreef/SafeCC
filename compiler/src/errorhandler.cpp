#include <core.h>
#include <errorhandler.h>
#include <symbols.h>
#include <token.h>
#include <types.h>

ErrorHandler err = ErrorHandler();

ErrorHandler::ErrorHandler()
{
}

void ErrorHandler::setupLinehandler(Scanner &scanner)
{
    m_scanner = &scanner;
}

void ErrorHandler::write(string str)
{
    std::cerr << str << "\n";
}

void ErrorHandler::lineError(struct ErrorInfo errInfo, string hl_color)
{
    if (errInfo.func == "")
        write("in the global scope of " + HL("'" + errInfo.file + "'"));
    else
        write("in function " + HL("'" + errInfo.func + "'") + " declared in " +
              HL("'" + errInfo.file + "'"));

    write("On line " + HL(to_string(errInfo.lineNum)) + " column " +
          HL(to_string(errInfo.charNum)));
    
    int tokstart = errInfo.charNum;
    if (tokstart)
        tokstart--;
        
    int tokend = errInfo.line.size();
    write("");
    write(to_string(errInfo.lineNum) + "| " + errInfo.line.substr(0, tokstart) + hl_color +
          errInfo.line.substr(tokstart, errInfo.line.size()) + ESCAPE_END);
    write(string(tokstart + to_string(errInfo.lineNum).size() + 2, ' ') + hl_color +
          string(tokend - tokstart, '^') + ESCAPE_END);

    m_justLoadedInfo = false;
}

void ErrorHandler::lineError(int line, int c, string hl_color)
{
    if (!m_justLoadedInfo)
        m_errInfo = createErrorInfo(line, c);
    lineError(m_errInfo, hl_color);
}

void ErrorHandler::lineError(string hl_color)
{
    if (!m_justLoadedInfo)
        m_errInfo = createErrorInfo();
    lineError(m_errInfo, hl_color);
}

void debughandler(int sig);
void ErrorHandler::fatal(string str)
{
    write(FATAL_PREFIX + str);
    lineError(ESCAPE_RED);

    write("Compilation ended");
#ifdef MODE_DEBUG
    debughandler(0);
#endif

    exit(1);
}

void ErrorHandler::fatal(string str, int l, int c)
{
    write(FATAL_PREFIX + str);
    lineError(l, c, ESCAPE_RED);
    write("Compilation ended");
    exit(1);
}

void ErrorHandler::fatalNL(string str)
{
    write(FATAL_PREFIX + str);
    write("Compilation ended");
    exit(1);
}

void ErrorHandler::warningNL(string str)
{
    if (f_warningAsError)
    {
        noticeNL("Treating warnings as errors because of --Werror flag");
        fatal(str);
    }
    write(WARNING_PREFIX + str);
}

void ErrorHandler::warning(string str)
{
    warningNL(str);
    lineError(ESCAPE_PURPLE);
}

void ErrorHandler::noticeNL(string str)
{
    write(NOTICE_PREFIX + str);
}

void ErrorHandler::notice(string str)
{
    noticeNL(str);
    lineError(ESCAPE_BLUE);
}

/* Cannot recover from syntax errors */
void ErrorHandler::syntaxError(string str)
{
    fatal("Syntax error: " + str);
}

void ErrorHandler::unexpectedToken(int token)
{
    fatal("Unexpected token: " + tokToStr(token));
}

void ErrorHandler::expectedToken(int token)
{
    fatal("Expected token " + tokToStr(token));
}

void ErrorHandler::expectedToken(int token, int got)
{
    fatal("Expected token: '" + tokToStr(token) + "' but instead got: '" +
          tokToStr(got) + "'");
}

void ErrorHandler::expectedToken(int token, int token2, int got)
{
    fatal("Expected token: '" + tokToStr(token) + "' or token '" +
          tokToStr(token2) + "' but instead got: '" + tokToStr(got) + "'");
}

void ErrorHandler::conversionWarning(struct Type *l, struct Type *r)
{
    if (f_conversionWarn)
        warning("Converting " + typeString(l) + " to " + typeString(r));
}

void ErrorHandler::ptrConversionWarning(struct Type *l, struct Type *r)
{
    if (f_ptrConversionWarn)
        warning("Converting " + typeString(l) + " to " + typeString(r));
}

void ErrorHandler::expectedType(struct Type *l, struct Type *r)
{
    // @wth: why do we need to string() this highlight ???
    fatal("Expected type: " + HL("'" + typeString(l) + "'") +
          " but instead got " + HL("'" + typeString(r) + "'"));
}

void ErrorHandler::unexpectedParamToFunc(string func, struct Type *t)
{
    fatal("Unexpected parameter to function: " + HL("'" + func + "'") +
          " of type: " + HL(typeString(t)));
}

void ErrorHandler::typeConversionError(struct Type *l, struct Type *r)
{
    fatal("Cannot convert " + HL(typeString(l)) + " to " + HL(typeString(r)) +
          " without a cast");
}

void ErrorHandler::unknownType(struct Type *l)
{
    fatal("Unknown type: " + HL(typeString(l)));
}

void ErrorHandler::unknownType(string s)
{
    fatal("Unknown type: " + HL("'" + s + "'"));
}

void ErrorHandler::unknownSymbol(string s)
{
    fatal("Unknown symbol " + HL("'" + s + "'"));
}

void ErrorHandler::tipNL(string s)
{
    write(TIP_PREFIX + s);
}

void ErrorHandler::tip(string s)
{
    write(TIP_PREFIX + s);
    lineError(ESCAPE_GREEN);
}

void ErrorHandler::unknownStructItem(string s, struct Type t)
{
    fatal("Unknown struct item: " + HL("'" + s + "'") + " in struct " +
          HL("'" + *t.name + "'"));
}

void ErrorHandler::incorrectAccessor(bool ptr)
{
    err.fatal("Invalid " + HL((ptr ? "'->'" : "'.'")) +
              " accessor to struct, did you mean to use " +
              HL((ptr ? "'.'" : "'->'")) + "?");
}

void ErrorHandler::pedanticWarning(string s)
{
    err.warning(s);
}

struct ErrorInfo ErrorHandler::createErrorInfo()
{
    return createErrorInfo(m_scanner->token().endLine(),
                            m_scanner->token().endCol());
}

struct ErrorInfo ErrorHandler::createErrorInfo(int line, int c)
{
    struct ErrorInfo ret;
    ret.tok     = m_scanner->token();
    ret.line    = m_scanner->curStrLine(m_scanner->curOffset() - c);
    ret.lineNum = line;
    ret.charNum = c;
    ret.func    = m_scanner->curFunction();
    ret.file    = m_scanner->curPPFile();

    return ret;
}

void ErrorHandler::loadErrorInfo(struct ErrorInfo ei)
{
    m_errInfo        = ei;
    m_justLoadedInfo = true;
}

void ErrorHandler::unloadErrorInfo()
{
    m_justLoadedInfo = false;
}

void ErrorHandler::loadedErrorInfo()
{
    m_justLoadedInfo = true;
}

struct ErrorInfo ErrorHandler::getLoadedErrorInfo()
{
    return m_errInfo;
}

void ErrorHandler::memWarn(string s)
{
    err.warning("Memory warnign: " + s);
}