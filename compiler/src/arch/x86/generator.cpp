#include <arch/x86/generator.h>
#include <errorhandler.h>
#include <symbols.h>
#include <types.h>

/* Helper functions */
int _sizeToDataSize(int size)
{
    switch (size)
    {
    case CHAR_SIZE:
        return 0;
    case SHORT_SIZE:
        return 1;
    case INT_SIZE:
        return 2;
    case LONGLONG_SIZE:
        return 3;
    default:
        err.fatalNL("Unsupported data size: " + size);
    }
}

int _regFromSize(int size)
{
    switch (size)
    {
    case CHAR_SIZE:
        return 4;
    case SHORT_SIZE:
        return 2;
    case INT_SIZE:
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
        return INT_SIZE;
    case 2:
        return SHORT_SIZE;
    case 3:
        return CHAR_SIZE;
    case 4:
        return CHAR_SIZE;
    }

    return 0;
}

GeneratorX86::GeneratorX86(string outfile) : Generator(outfile)
{
    freeAllReg();
    fprintf(m_outfile, "section .text\nglobal main\n");
}

string GeneratorX86::getReg(int r)
{
    if (!m_usedRegisters[r])
    {
        err.warningNL("Register: " + m_dwordRegisters[r] + " is unused");
        return m_dwordRegisters[r];
    }
    else
    {
        return m_registers[m_usedRegisters[r] - 1][r];
    }
}

void GeneratorX86::freeAllReg()
{
    m_spilledRegisters = 0;
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
        m_usedRegisters[i] = 0;
}

void GeneratorX86::spillReg(int reg)
{
    write("push", getReg(reg));
}

void GeneratorX86::loadReg(int reg)
{
    write("pop", getReg(reg));
}

void GeneratorX86::freeReg(int reg)
{
    if (m_spilledRegisters && (m_spilledRegisters - 1) % REGAMOUNT == reg)
    {
        loadReg(reg);
        m_spilledRegisters--;
        return;
    }
    
    if (m_usedRegisters[reg] == 0)
    {
        err.warningNL("Trying to free a register that is already free: " +
                      m_dwordRegisters[reg]);
    }
    m_usedRegisters[reg] = 0;
}

int GeneratorX86::allocReg()
{
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
    {
        if (!m_usedRegisters[i])
        {
            /* Allocate dword register */
            m_usedRegisters[i] = 1;
            return i;
        }
    }
    
    int reg = m_spilledRegisters % REGAMOUNT;
    m_spilledRegisters++;
    spillReg(reg);
    
    return reg;
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
    write("mov", getReg(r), getReg(r2));
    return r2;
}

int GeneratorX86::genFunctionPreamble(int funcIdx)
{
    // Clean all the registers
    freeAllReg();

    Symbol *s = g_symtable.getSymbol(funcIdx);

    if (s->storageClass == SymbolTable::StorageClass::EXTERN)
        fprintf(m_outfile, "global %s\n", s->name.c_str());

    fprintf(m_outfile, "%s:\n", s->name.c_str());
    write("push", "ebp");
    write("mov", "esp", "ebp");
    write("sub", s->localVarAmount + 4, "esp");
    return -1;
}

int GeneratorX86::genFunctionPostamble(int funcIdx)
{
    int            l;
    Symbol *s = g_symtable.getSymbol(funcIdx);
    if ((l = s->returnLabelId) != -1)
        genLabel(l);

    write("leave");

    if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
        write("ret 0x4");
    else
        write("ret");
    return -1;
}

int GeneratorX86::genLoad(int value, int size)
{
    int reg              = allocReg();
    m_usedRegisters[reg] = _regFromSize(size);
    write("mov", value, getReg(reg));
    return reg;
}

int GeneratorX86::genAdd(int r1, int r2)
{
    write("add", getReg(r2), getReg(r1));
    freeReg(r2);
    return r1;
}

int GeneratorX86::genSub(int r1, int r2)
{
    write("sub", getReg(r2), getReg(r1));
    freeReg(r2);
    return r1;
}

int GeneratorX86::genMul(int r1, int r2)
{
    write("imul", getReg(r2), getReg(r1));
    freeReg(r2);
    return r1;
}

int GeneratorX86::_genIDiv(int r1, int r2, bool quotient)
{
    DEBUG("DIV")
    bool r3 = false;
    bool r4 = false;
    
    if (m_usedRegisters[EAX] && r1 != EAX)
    {
        r3 = true;
        write("push", "eax");
        write("mov", getReg(r1), "eax");
    }
    
    if (m_usedRegisters[EDX])
    {
        r4 = true;
        write("push", "edx");
    }

    write("push", getReg(r2));
    freeReg(r2);

    write("xor", "edx", "edx");
    move("mov", getReg(r1), "eax");
    write("cdq");
    write("idiv", "dword [esp]");
    
    string ret = "edx";
    if (quotient)
        ret = "eax";
    move("mov", ret, getReg(r1));
    write("add", 4, "esp");

    if (r4)
        write("pop", "edx");

    if (r3)
        write("pop", "eax");
    
    return r1;
}

int GeneratorX86::genDiv(int r1, int r2)
{
    return _genIDiv(r1, r2, true);
}

string variableAccess(int symbol, int offset = 0)
{
    Symbol *s = g_symtable.getSymbol(symbol);

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
            int varSize = getTypeSize(*s);
            if (s->varType.typeType == TypeTypes::STRUCT &&
                !s->varType.ptrDepth)
                offset += s->varType.size - 4;

            else if (s->varType.isArray /*&& !(s->varType.ptrDepth - 1) */)
                offset += varSize - 4;

            return "ebp-" + to_string(s->stackLoc + 4 + offset);
        }
    }
    else
    {
        return s->name + (offset ? ("+" + to_string(offset)) : "");
    }
}

int GeneratorX86::genLoadVariable(int symbol, Type t)
{
    int reg = allocReg();

    // Clear the register if it is smaller then a DWORD
    // if (regs != 1)
    //    write("xor", m_dwordRegisters[reg], m_dwordRegisters[reg]);
    
    Symbol *s = g_symtable.getSymbol(symbol);

    if (s->varType.isArray)
    {
        m_usedRegisters[reg] = 1;
        write("lea", MEMACCESS(variableAccess(symbol)), getReg(reg));
    }
    else if (t.typeType == TypeTypes::STRUCT && !t.ptrDepth)
    {
        write("lea", MEMACCESS(variableAccess(symbol)), getReg(reg));
    }
    else
    {
        int regs             = _regFromSize(t.size);
        m_usedRegisters[reg] = regs;
        write("mov", MEMACCESS(variableAccess(symbol)), getReg(reg));
    }

    return reg;
}

int GeneratorX86::genStoreValue(int reg1, int memloc, Type t)
{
    if (t.typeType == TypeTypes::STRUCT && !t.ptrDepth)
    {
        for (struct StructItem s : t.contents)
        {
            // mov [reg1 + offset] -> tmp
            // mov tmp -> [memloc + offset]symType ==
            // SymbolTable::SymTypes::ARRAY
            int tmp = allocReg();
            write("mov", "[" + getReg(reg1) + "+" + to_string(s.offset) +"]", getReg(tmp));
            write("mov", getReg(tmp), "[" + getReg(memloc) + "+" + to_string(s.offset) +"]");
            freeReg(tmp);
        }
    }
    else
        write("mov", getReg(reg1), MEMACCESS(getReg(memloc)));
    freeReg(memloc);
    return reg1;
}

int GeneratorX86::genExternSection()
{
    fprintf(m_outfile, "\n");
    for (Symbol s : g_symtable.getGlobalTable())
    {
        if (s.storageClass == SymbolTable::StorageClass::EXTERN)
        {
            if (s.used)
            {
                if (!s.defined)
                    fprintf(m_outfile, "extern %s\n", s.name.c_str());

                else if (s.symType == SymbolTable::SymTypes::VARIABLE)
                    fprintf(m_outfile, "global %s\n", s.name.c_str());
            }
        }
    }
}

int GeneratorX86::genDataSection()
{
    genExternSection();
    
    fprintf(m_outfile, "\n\nsection\t.data\n");
    for (Symbol s : g_symtable.getGlobalTable())
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
            Type t = s.varType;
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
static string jmpinstr[] = {"je", "jne", "jl", "jg", "jle", "jge"};

int GeneratorX86::genCompare(int reg1, int reg2, bool clearReg)
{
    write("cmp", getReg(reg2), getReg(reg1));
    freeReg(reg2);
    
    if (!clearReg)
        return reg1;
    
    freeReg(reg1);
    return -1;
}
int GeneratorX86::genFlagJump(int op, int label)
{
    write(jmpinstr[op - AST::Types::EQUAL], LABEL(label));
    return -1;
}

int GeneratorX86::genCompareSet(int op, int reg1, int reg2)
{
    write("cmp", getReg(reg2), getReg(reg1));
    
    // mov reg2, 0 is used here instead of xor reg2, reg2 because
    // xor trashes the ZF flag in eflags and thus the result of the cmp 
    write("mov", 0, getReg(reg2));
    
    write(setinstr[op - AST::Types::EQUAL], m_loByteRegisters[reg2]);
    
    write("movzx", m_loByteRegisters[reg2], getReg(reg2));
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
    case SHORT_SIZE:
        newreg = 1;
        break;
    case INT_SIZE:
        newreg = 0;
        break;
    default:
        err.fatalNL("Unsupported 'new' operant size: " + to_string(newsize));
    }

    if (isSigned)
        write("movsx", getReg(reg), m_registers[newreg][reg]);

    else
        write("movzx", getReg(reg), m_registers[newreg][reg]);

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
vector <int> GeneratorX86::genSaveRegisters()
{
    vector <int> pushedRegisters;
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
    {
        if (m_usedRegisters[i])
            spillReg(i);
        
        pushedRegisters.push_back(m_usedRegisters[i]);
        m_usedRegisters[i] = 0;
    }
    
    return pushedRegisters;
}

int GeneratorX86::genLoadRegisters(vector<int> data)
{
    for (int i = data.size() - 1; i >= 0; i--)
    {
        if (data[i])
            loadReg(i);
        m_usedRegisters[i] = data[i];
    }
    
    return -1;
}
bool GeneratorX86::hasFreeReg()
{
    for (int i = 0; i < SIZE(m_usedRegisters); i++)
        if (!m_usedRegisters[i])
            return true;
    
    return false;
}

int GeneratorX86::genFunctionCall(int symbolidx, int parameters, vector<int> data)
{

    Symbol *s = g_symtable.getSymbol(symbolidx);

    if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
    {
        write("sub", s->varType.size, "esp");
        write("push", "esp");
        data = genSaveRegisters();
    }


    write("call", s->name);
    /**
     * cdecl states that the caller should clean the stack so let's be nice
     * and do so
     */
    write("add", parameters * 4, "esp");
    for (int i = 0; i < data.size(); i++)
        m_usedRegisters[i] = data[i];
    
    int out = EAX;
    int offset = 0;
    
    if (s->varType.primType != PrimitiveTypes::VOID)
    {
        if(m_usedRegisters[EAX])
        {
            if (!hasFreeReg())
            {
                write("push", "eax");
                offset = 4;
            }
            else
            {
                out = allocReg();
                write("mov", "eax", getReg(out));
            }
        }
        else
        {
            if (s->varType.typeType == TypeTypes::STRUCT)
                m_usedRegisters[out] = 1;
            else
                m_usedRegisters[out] = _regFromSize(s->varType.size);
        }
        
    }
    
    int pushAmount = 0;
    for (int i = 0; i < data.size(); i++) 
        if (data[i]) 
            pushAmount++;
    
    for (int i = data.size() -1; i >= 0; i--)
    {
        if (data[i])
        {
            write("mov", "[esp+" + to_string(offset + ((pushAmount - i) * 4) - 4) + "]",
                  getReg(i));
        }
    }
    
    if (offset)
    {
        out = allocReg();
        write("mov", "[esp+" + to_string(offset + 4) + "]", getReg(out));
    }
        
    return out;
}

int GeneratorX86::genReturnJump(int reg, int funcIdx)
{
    if (g_symtable.getSymbol(funcIdx)->returnLabelId == -1)
        g_symtable.getSymbol(funcIdx)->returnLabelId = label();

    Symbol *s = g_symtable.getSymbol(funcIdx);

    if (reg == -1)
        return genJump(g_symtable.getSymbol(funcIdx)->returnLabelId);

    if (s->varType.typeType == TypeTypes::STRUCT && !s->varType.ptrDepth)
    {
        int ptrReg = allocReg();
        int tmpReg = allocReg();
        write("mov", "dword [ebp + 8]", getReg(ptrReg));
        for (struct StructItem s : s->varType.contents)
        {
            string size = SPECIFYSIZE(_regFromSize(s.itemType.size));
            write("mov",
                  size + MEMACCESS(getReg(reg) + "+" + to_string(s.offset)),
                  getReg(tmpReg));
            write("mov", getReg(tmpReg),
                  size + MEMACCESS(getReg(ptrReg) + "+" + to_string(s.offset)));
        }
        move("mov", getReg(ptrReg), "eax");
        freeReg(tmpReg);
    }
    else
    {
        move("mov", getReg(reg), "eax");
    }
    freeReg(reg);
    allocReg(EAX);

    genJump(g_symtable.getSymbol(funcIdx)->returnLabelId);
}

int GeneratorX86::genLoadLocation(int symbolidx)
{
    Symbol *s   = g_symtable.getSymbol(symbolidx);
    int            reg = allocReg();

    write("lea", MEMACCESS(variableAccess(symbolidx)), getReg(reg));

    return reg;
}

int GeneratorX86::genPtrAccess(int memreg, int size)
{
    int reg = allocReg();
    
    // If we have something like a struct, set the size to PTR size
    if (size > PTR_SIZE)
        size = PTR_SIZE;
    m_usedRegisters[reg] = _regFromSize(size);

    write("mov",
          SPECIFYSIZE(m_usedRegisters[reg]) + MEMACCESS(m_dwordRegisters[memreg]),
          getReg(reg));
    freeReg(memreg);
    return reg;
}

int GeneratorX86::genDirectMemLoad(int offset, int symbol, int reg, int size)
{
    Symbol *s = g_symtable.getSymbol(symbol);
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
    write("mov", getReg(reg), SPECIFYSIZE(_regFromSize(size)) + MEMACCESS(str));

    freeReg(reg);

    return -1;
}

int GeneratorX86::genNegate(int reg)
{
    write("neg", getReg(reg));
    return reg;
}

int GeneratorX86::genAccessStruct(int memreg, int offset, int size)
{
    int reg = allocReg();
    write("mov", SPECIFYSIZE(_regFromSize(size)) + MEMACCESS(getReg(memreg) + "+" + to_string(offset)), getReg(reg));
    return reg;
}

int GeneratorX86::genIncrement(int symbol, int amount, int after)
{
    int reg = genLoadVariable(symbol, g_symtable.getSymbol(symbol)->varType);
    int saveReg = -1;
    
    if (after)
    {
        saveReg = allocReg();
        m_usedRegisters[saveReg] = m_usedRegisters[reg];
        write("mov", getReg(reg), getReg(saveReg));
    }
    
    if (amount == 1)
        write("inc", getReg(reg));
    else
        write("add", amount, getReg(reg));
    
    int s = m_usedRegisters[reg];
    write("mov", getReg(reg), SPECIFYSIZE(s)+MEMACCESS(variableAccess(symbol)));
        
    if (after)
    {
        freeReg(reg);
        return saveReg;
    }
    
    return reg;
}

int GeneratorX86::genDecrement(int symbol, int amount, int after)
{
    int reg = genLoadVariable(symbol, g_symtable.getSymbol(symbol)->varType);
    int saveReg = -1;
    
    if (amount == 1)
        write("dec", getReg(reg));
    else
        write("sub", amount, getReg(reg));
        
    
    if (after)
    {
        saveReg = allocReg();
        m_usedRegisters[saveReg] = m_usedRegisters[reg];
        write("mov", getReg(reg), getReg(saveReg));
    }
    
    write("mov", getReg(reg), SPECIFYSIZE(reg)+MEMACCESS(variableAccess(symbol)));
        
    if (after)
    {
        freeReg(reg);
        return saveReg;
    }
    
    return reg;
}
int GeneratorX86::genLeftShift(int reg, int amount)
{
    allocReg(ECX);
    move("mov", getReg(amount), "ecx");
    freeReg(amount);
    write("shl", "cl", getReg(reg));
    return reg;
}

int GeneratorX86::genRightShift(int reg, int amount)
{
    allocReg(ECX);
    move("mov", getReg(amount), "ecx");
    freeReg(amount);
    write("shr", "cl", getReg(reg));
    return reg;
}

int GeneratorX86::genModulus(int r1, int r2)
{
    return _genIDiv(r1, r2, false);
}

void GeneratorX86::genDebugComment(string comment)
{
    int end = comment.length();
    
    for (int i = 0; i < end; i++)
    {
        if (comment[i] == '\n')
        {
            comment.insert(++i, "; ");
            end++;
        }
    }
    
    fprintf(m_outfile, "; %s\n", comment.c_str());
}

int GeneratorX86::genAnd(int reg1, int reg2)
{
    write("and", getReg(reg2), getReg(reg1));
    freeReg(reg2);
    return reg1;
}

int GeneratorX86::genOr(int reg1, int reg2)
{
    write("or", getReg(reg2), getReg(reg1));
    freeReg(reg2);
    return reg1;
}

int GeneratorX86::genXor(int reg1, int reg2)
{
    write("xor", getReg(reg2), getReg(reg1));
    freeReg(reg2);
    return reg1;
}

int GeneratorX86::genBinNegate(int reg1)
{
    write("not", getReg(reg1));
    return reg1;
}

int GeneratorX86::genIsZero(int reg)
{
    write("test", getReg(reg), getReg(reg));
    freeReg(reg);
    return -1;
}

int GeneratorX86::genLogAnd(int reg1, int reg2)
{
    write("and", getReg(reg2), getReg(reg1));
    freeReg(reg2);
    return reg1;
}
int GeneratorX86::genLogOr(int reg1, int reg2)
{
    write("or", getReg(reg2), getReg(reg1));
    freeReg(reg2);
    return reg1;
}

int GeneratorX86::genIsZeroSet(int reg1, bool setOnZero)
{
    int reg2 = allocReg();
    write("xor", getReg(reg2), getReg(reg2));
    write("test", getReg(reg1), getReg(reg1));
    write(setinstr[!setOnZero], m_loByteRegisters[reg2]);
    write("movzx", m_loByteRegisters[reg2], getReg(reg2));
    freeReg(reg1);
    return reg2;
}

int GeneratorX86::genLabel(string label)
{
    fprintf(m_outfile, "%s\n", ("." + label + ":").c_str());
    return -1;
}

int GeneratorX86::genGoto(string label)
{
    write("jmp", "." + label);
    return -1;
}

int GeneratorX86::genMoveReg(int reg, int toReg)
{
    // We don't need to move the register in to a new one if it isn't specified
    if (toReg == -1)
        return reg;
        
    if (toReg == reg)
        return toReg;
        
    write("mov", getReg(reg), getReg(toReg));
    freeReg(reg);
    return toReg;
}