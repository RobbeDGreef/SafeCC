#include <arch/x86/generator.h>
#include <errorhandler.h>
#include <symbols.h>
#include <types.h>

/* Helper nctions */
int _sizeToDataSize(int size)
{
    switch (size)
    {
    case BYTE:
        return 0;
    case WORD:
        return 1;
    case DWORD:
        return 2;
    case QWORD:
        return 3;
    default:
        err.fatalNL("Unsupported data size: " + size);
    }
}

int _regFromSize(int size)
{
    switch (size)
    {
    case BYTE:
        return 4;
    case WORD:
        return 2;
    case DWORD:
        return 1;
    default:
        err.warning("Could not translate operant size to register size (" +
                    to_string(size) + ")");
        return 1;
    }
}

int _dataSizeFromRegSize(int regsize)
{
    switch (regsize)
    {
    case 1:
        return 32;
    case 2:
        return 16;
    case 3:
        return 8;
    case 4:
        return 8;
    }

    return 0;
}

GeneratorX86::GeneratorX86(string outfile) : Generator(outfile)
{
    freeAllReg();
    fprintf(m_outfile, "section .text\nglobal main\n");
}

void GeneratorX86::freeAllReg()
{
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
        m_usedRegisters[i] = 0;
}

void GeneratorX86::freeReg(int reg)
{
    if (m_usedRegisters[reg] == 0)
    {
        err.warningNL("Trying to free a register that is already free: " +
                      GETREG(reg));
    }
    m_usedRegisters[reg] = 0;
}

int GeneratorX86::allocReg()
{
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
    {
        if (!m_usedRegisters[i])
        {
            /* Alocate dword register */
            m_usedRegisters[i] = 1;
            return i;
        }
    }
    err.fatalNL("Ran out of usable registers");
}

/**
 * @brief   Will allocate a specific register (will free it and return it
 * otherwise it will throw an error)
 *
 * @param   r the register to allocate (index)
 *
 * @return  @todo: explain this
 */
int GeneratorX86::allocReg(int r)
{
    if (!m_usedRegisters[r])
    {
        m_usedRegisters[r] = 1;
        return -1;
    }

    int r2 = allocReg();
    write("mov", GETREG(r), GETREG(r2));
    return r2;
}

int GeneratorX86::genFunctionPreamble(int funcIdx)
{
    // Clean all the registers
    freeAllReg();

    fprintf(m_outfile, "%s:\n", g_symtable.getSymbol(funcIdx)->name.c_str());
    write("push", "ebp");
    write("mov", "esp", "ebp");
    write("sub", g_symtable.getSymbol(funcIdx)->localVarAmount, "esp");
    return -1;
}

int GeneratorX86::genFunctionPostamble(int funcIdx)
{
    int            l;
    struct Symbol *s = g_symtable.getSymbol(funcIdx);
    if ((l = s->returnLabelId) != -1)
        genLabel(l);

    write("leave");

    if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
        write("ret 0x4");
    else
        write("ret");
    return -1;
}

int GeneratorX86::move(string instr, string source_reg, string dest_reg)
{
    /* Little optimization, do not move same reg in same reg */
    if (source_reg.compare(dest_reg))
        write(instr, source_reg, dest_reg);
}

int GeneratorX86::genLoad(int value, int size)
{
    DEBUG("Load " << value)
    int reg              = allocReg();
    m_usedRegisters[reg] = _regFromSize(size);
    write("mov", value, GETREG(reg));
    return reg;
}

int GeneratorX86::genAdd(int r1, int r2)
{
    write("add", GETREG(r2), GETREG(r1));
    freeReg(r2);
    return r1;
}

int GeneratorX86::genSub(int r1, int r2)
{
    write("sub", GETREG(r2), GETREG(r1));
    freeReg(r2);
    return r1;
}

int GeneratorX86::genMul(int r1, int r2)
{
    write("imul", GETREG(r2), GETREG(r1));
    freeReg(r2);
    return r1;
}

int GeneratorX86::genDiv(int r1, int r2)
{
    DEBUG("DIV")
    int r3 = -1;
    int r4 = -1;

    write("push", GETREG(r2));
    freeReg(r2);

    if (m_usedRegisters[EAX] && r1 != EAX)
    {
        /* Allocate register eax */
        r3 = allocReg(EAX);
    }

    if (m_usedRegisters[EDX])
    {
        /* Allocate register edx */
        r4 = allocReg(EDX);
    }

    write("xor", "edx", "edx");
    move("mov", GETREG(r1), "eax");
    write("cdq");
    write("idiv", "dword [esp]");
    move("mov", "eax", GETREG(r1));

    if (r3 >= 0)
    {
        write("mov", GETREG(r3), "eax");
        freeReg(r3);
    }

    if (r4 >= 0)
    {
        write("mov", GETREG(r4), "edx");
        freeReg(r4);
    }

    write("add", 4, "esp");

    DEBUG("ENDDIV")
    return r1;
}

string variableAccess(int symbol, int offset = 0)
{
    struct Symbol *s = g_symtable.getSymbol(symbol);

    // The variable is a local variable if the lower 8 bits of symbol contain a
    // value
    if (symbol & 0xFF && s->storageClass != SymbolTable::StorageClass::EXTERN)
    {
        if (s->symType == SymbolTable::SymTypes::ARGUMENT)
        {
            return "ebp+" + to_string(s->stackLoc * 4 + 8 + offset);
        }
        else
        {
            if (s->varType.typeType == TypeTypes::STRUCT &&
                !s->varType.ptrDepth)
                offset += s->varType.size -
                          s->varType.contents.back().itemType.size / 8;

            else if (s->varType.isArray && !(s->varType.ptrDepth - 1))
                offset += (s->varType.size / 8) * s->value - 4;

            return "ebp-" + to_string(s->stackLoc + 4 + offset);
        }
    }
    else
    {
        return s->name + (offset ? ("+" + to_string(offset)) : "");
    }
}

int GeneratorX86::genLoadVariable(int symbol)
{
    int reg = allocReg();

    // Clear the register if it is smaller then a DWORD
    // if (regs != 1)
    //    write("xor", m_dwordRegisters[reg], m_dwordRegisters[reg]);

    struct Symbol *s = g_symtable.getSymbol(symbol);
    if (s->varType.isArray)
    {
        m_usedRegisters[reg] = 1;
        write("lea", MEMACCESS(variableAccess(symbol)), GETREG(reg));
    }
    else if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
    {
        write("lea", MEMACCESS(variableAccess(symbol)), GETREG(reg));
    }
    else
    {
        int regs             = _regFromSize(s->varType.size);
        m_usedRegisters[reg] = regs;
        write("mov", MEMACCESS(variableAccess(symbol)), GETREG(reg));
    }

    return reg;
}

int GeneratorX86::genStoreValue(int reg1, int memloc, struct Type t)
{
    DEBUG("reg1: " << reg1 << " mem " << memloc)

    if (t.typeType == TypeTypes::STRUCT && !t.ptrDepth)
    {
        for (struct StructItem s : t.contents)
        {
            // mov [reg1 + offset] -> tmp
            // mov tmp -> [memloc + offset]symType ==
            // SymbolTable::SymTypes::ARRAY
        }
    }
    else
        write("mov", GETREG(reg1), MEMACCESS(GETREG(memloc)));
    freeReg(reg1);
    freeReg(memloc);
    return -1;
}

int GeneratorX86::genExternSection()
{
    fprintf(m_outfile, "\n");
    for (struct Symbol s : g_symtable.getGlobalTable())
    {
        if (s.storageClass == SymbolTable::StorageClass::EXTERN)
        {
            if (s.used)
            {
                if (!s.defined)
                    fprintf(m_outfile, "extern %s\n", s.name.c_str());

                else
                    fprintf(m_outfile, "global %s\n", s.name.c_str());
            }
        }
    }
}

int GeneratorX86::genDataSection()
{
    genExternSection();
    
    fprintf(m_outfile, "\n\nsection\t.data\n");
    for (struct Symbol s : g_symtable.getGlobalTable())
    {
        if (s.symType == SymbolTable::SymTypes::VARIABLE &&
            s.varType.typeType != TypeTypes::STRUCT && !s.varType.isArray && 
            s.storageClass != SymbolTable::StorageClass::EXTERN)
            fprintf(m_outfile, "\t%s\t%s %d\n", s.name.c_str(),
                    m_initDataSize[_sizeToDataSize(s.varType.size)].c_str(),
                    s.value);

        else if (s.varType.isArray &&
                 s.symType == SymbolTable::SymTypes::VARIABLE &&
                 s.storageClass != SymbolTable::StorageClass::EXTERN)
        {
            struct Type t = s.varType;
            dereference(&t);

            fprintf(m_outfile, "\t%s\t%s ", s.name.c_str(),
                    m_initDataSize[_sizeToDataSize(t.size)].c_str());
            for (string s : s.inits)
            {
                fprintf(m_outfile, "%s, ", s.c_str());
            }

            if (s.inits.size() < s.value)
            {
                int amount = s.value - s.inits.size();
                fprintf(m_outfile, "times %u %s 0", amount,
                        m_initDataSize[_sizeToDataSize(t.size)].c_str());
            }

            fprintf(m_outfile, "\n");
        }
        else if (s.symType == SymbolTable::SymTypes::VARIABLE &&
                 s.varType.typeType == TypeTypes::STRUCT && 
                 s.storageClass != SymbolTable::StorageClass::EXTERN)
        {
            fprintf(m_outfile, "\t%s\n", s.name.c_str());

            for (int i = 0; i < s.varType.contents.size(); i++)
            {
                struct StructItem sItem = s.varType.contents[i];
                if (i < s.inits.size())
                    write("\t" + m_initDataSize[_sizeToDataSize(sItem.itemType.size)],
                          s.inits[i]);

                write("\t" + m_initDataSize[_sizeToDataSize(sItem.itemType.size)],
                      0);
            }
        }
    }
}

/* SETcc instructions */
static string setinstr[] = {"sete", "setne", "setl", "setg", "setle", "setge"};

/* Inverted jump instructions */
static string jmpinstr[] = {"jne", "je", "jge", "jle", "jg", "jl"};

int GeneratorX86::genCompareJump(int op, int reg1, int reg2, int label)
{
    write("cmp", GETREG(reg2), GETREG(reg1));
    write(jmpinstr[op - AST::Types::EQUAL], LABEL(label));
    freeReg(reg1);
    freeReg(reg2);
    return -1;
}

int GeneratorX86::genCompareSet(int op, int reg1, int reg2)
{
    write("cmp", GETREG(reg2), GETREG(reg1));
    write(setinstr[op - AST::Types::EQUAL], m_loByteRegisters[reg2]);
    write("movzx", m_loByteRegisters[reg2], GETREG(reg2));
    freeReg(reg1);
    return reg2;
}

int GeneratorX86::genJump(int label)
{
    write("jmp", LABEL(label));
    return -1;
}

int GeneratorX86::genLabel(int label)
{
    fprintf(m_outfile, ".L%d:\n", label);
    return -1;
}

int GeneratorX86::genWidenRegister(int reg, int oldsize, int newsize,
                                   bool isSigned)
{
    int newreg;
    DEBUG("widen reg: " << reg << " with oldsize: " << oldsize << " and new "
                        << newsize);

    switch (newsize)
    {
    case WORD:
        newreg = 1;
        break;
    case DWORD:
        newreg = 0;
        break;
    default:
        err.fatalNL("Unsupported 'new' operant size: " + to_string(newsize));
    }

    if (isSigned)
        write("movsx", GETREG(reg), m_registers[newreg][reg]);

    else
        write("movzx", GETREG(reg), m_registers[newreg][reg]);

    m_usedRegisters[reg] = newreg + 1;

    return reg;
}

int GeneratorX86::genPushArgument(int reg, int argindex)
{
    // argindex is not used on x86

    // Only double words can be pushed on x86

    // We don't want to widen the registers before pushing them because if we
    // would signedness would be destroyed when using non dword registers

    write("push", m_dwordRegisters[reg]);
    freeReg(reg);
    return -1;
}

int GeneratorX86::checkRegisters()
{
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
    {
        if (m_usedRegisters[i] != 0)
        {
            err.warningNL("Register: " + m_dwordRegisters[i] + " (" +
                          to_string(i) + ") is not free after functioncall ?");
        }
    }
}

int GeneratorX86::genFunctionCall(int symbolidx, int parameters)
{
    /* normally now all registers are clean */
    checkRegisters();
    struct Symbol *s = g_symtable.getSymbol(symbolidx);

    if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
    {
        write("sub", s->varType.size, "esp");
        int reg = allocReg();
        write("lea", "[esp]", GETREG(reg));
        write("push", GETREG(reg));
        freeReg(reg);
    }

    write("call", s->name);
    /**
     * cdecl states that the caller should clean the stack so let's be nice
     * and do so
     */
    write("add", parameters * 4, "esp");

    if (s->varType.primType != PrimitiveTypes::VOID)
    {
        if (s->varType.typeType == TypeTypes::STRUCT)
            m_usedRegisters[EAX] = 1;
        else
            m_usedRegisters[EAX] = _regFromSize(s->varType.size);
    }

    /* EAX is return value, and thus we return it */
    return EAX;
}

int GeneratorX86::genReturnJump(int reg, int funcIdx)
{
    if (g_symtable.getSymbol(funcIdx)->returnLabelId == -1)
        g_symtable.getSymbol(funcIdx)->returnLabelId = m_labelCount++;

    struct Symbol *s = g_symtable.getSymbol(funcIdx);

    if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
    {
        int ptrReg = allocReg();
        int tmpReg = allocReg();
        write("mov", "dword [ebp + 8]", GETREG(ptrReg));
        for (struct StructItem s : s->varType.contents)
        {
            string size = SPECIFYSIZE(_regFromSize(s.itemType.size));
            write("mov",
                  size + MEMACCESS(GETREG(reg) + "+" + to_string(s.offset)),
                  GETREG(tmpReg));
            write("mov", GETREG(tmpReg),
                  size + MEMACCESS(GETREG(ptrReg) + "+" + to_string(s.offset)));
        }
        move("mov", GETREG(ptrReg), "eax");
        freeReg(tmpReg);
    }
    else
    {
        move("mov", GETREG(reg), "eax");
    }
    freeReg(reg);
    allocReg(EAX);

    genJump(m_labelCount - 1);
}

int GeneratorX86::genLoadLocation(int symbolidx)
{
    struct Symbol *s   = g_symtable.getSymbol(symbolidx);
    int            reg = allocReg();

    write("lea", MEMACCESS(variableAccess(symbolidx)), GETREG(reg));

    return reg;
}

int GeneratorX86::genPtrAccess(int memreg, int size)
{
    int reg = allocReg();
    /* Set the allocated registers size */
    m_usedRegisters[reg] = _regFromSize(size);

    write("mov",
          SPECIFYSIZE(m_usedRegisters[reg]) + MEMACCESS(m_dwordRegisters[memreg]),
          GETREG(reg));
    freeReg(memreg);
    return reg;
}

int GeneratorX86::genDirectMemLoad(int offset, int symbol, int reg, int size)
{
    struct Symbol *s = g_symtable.getSymbol(symbol);
    string         str;

    // Check whether a variable is local or not
    if (symbol & 0xFF)
    {
        str = "ebp-" + to_string(s->stackLoc + 4 + offset);
    }
    else
    {
        str = s->name + "+" + to_string(offset);
    }
    write("mov", GETREG(reg), SPECIFYSIZE(_regFromSize(size)) + MEMACCESS(str));

    freeReg(reg);

    return -1;
}

int GeneratorX86::genNegate(int reg)
{
    write("neg", GETREG(reg));
    return reg;
}

int GeneratorX86::genAccessStruct(int symbol, int idx)
{
    DEBUG("sym: " << symbol << " " << idx)
    struct Symbol *s     = g_symtable.getSymbol(symbol);
    int            reg   = allocReg();
    int            size  = s->varType.contents[idx].itemType.size;
    m_usedRegisters[reg] = _regFromSize(size);

    int offset = s->varType.contents[idx].offset;
    write("mov",
          SPECIFYSIZE(_regFromSize(size)) + MEMACCESS(variableAccess(symbol,
                                                                     offset)),
          GETREG(reg));

    return reg;
}