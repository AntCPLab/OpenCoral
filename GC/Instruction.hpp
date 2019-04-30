/*
 * Instruction.cpp
 *
 */

#include <algorithm>

#include "GC/Instruction.h"
#include "GC/Processor.h"

#include "Processor/Instruction.h"

#include "Tools/parse.h"

#include "GC/Instruction_inline.h"

namespace GC
{

template <class T>
Instruction<T>::Instruction() :
        BaseInstruction()
{
    code = fallback_code;
    size = 1;
}

template <class T>
bool Instruction<T>::get_offline_data_usage(int& usage)
{
    switch (opcode)
    {
    case ::USE:
        usage += n;
        return int(n) >= 0;
    default:
        return true;
    }
}

template <class T>
int Instruction<T>::get_reg_type() const
{
    switch (opcode & 0x2F0)
    {
    case SECRET_WRITE:
        return SBIT;
    case CLEAR_WRITE:
        return CBIT;
    default:
        switch (::BaseInstruction::get_reg_type())
        {
        case ::INT:
            return INT;
        case ::MODP:
            switch (opcode)
            {
            case LDMC:
            case STMC:
            case XORC:
            case ADDC:
            case ADDCI:
            case MULCI:
            case SHRCI:
            case SHLCI:
                return CBIT;
            }
            return SBIT;
        }
        return NONE;
    }
}

template<class T>
unsigned GC::Instruction<T>::get_max_reg(int reg_type) const
{
    int skip;
    int offset = 0;
    switch (opcode)
    {
    case LDMSD:
    case LDMSDI:
        skip = 3;
        break;
    case STMSD:
    case STMSDI:
        skip = 2;
        break;
    case ANDRS:
    case XORS:
    case ANDS:
        skip = 4;
        offset = 1;
        break;
    case INPUTB:
        skip = 3;
        offset = 2;
        break;
    case CONVCBIT:
        return BaseInstruction::get_max_reg(INT);
    default:
        return BaseInstruction::get_max_reg(reg_type);
    }

    unsigned m = 0;
    if (reg_type == SBIT)
        for (size_t i = offset; i < start.size(); i += skip)
            m = max(m, (unsigned)start[i] + 1);
    return m;
}

template <class T>
unsigned Instruction<T>::get_mem(RegType reg_type) const
{
    unsigned m = n + 1;
    switch (opcode)
    {
    case LDMSD:
        if (reg_type == DYN_SBIT)
        {
            m = 0;
            for (size_t i = 0; i < start.size() / 3; i++)
                m = max(m, (unsigned)start[3*i+1] + 1);
            return m;
        }
        break;
    case STMSD:
        if (reg_type == DYN_SBIT)
        {
            m = 0;
            for (size_t i = 0; i < start.size() / 2; i++)
                m = max(m, (unsigned)start[2*i+1] + 1);
            return m;
        }
        break;
    case LDMS:
    case STMS:
        if (reg_type == SBIT)
            return m;
        break;
    case LDMC:
    case STMC:
        if (reg_type == CBIT)
            return m;
        break;
    case LDMINT:
    case STMINT:
        if (reg_type == INT)
            return m;
        break;
    }
    return 0;
}

template <class T>
void Instruction<T>::parse(istream& s, int pos)
{
    n = 0;
    start.resize(0);
    ::memset(r, 0, sizeof(r));

    int file_pos = s.tellg();
    opcode = ::get_int(s);

    try {
        parse_operands(s, pos);
    }
    catch (Invalid_Instruction& e)
    {
        int m;
        switch (opcode)
        {
        case XORM:
            n = get_int(s);
            get_ints(r, s, 3);
            break;
        case XORCI:
        case MULCI:
        case LDBITS:
            get_ints(r, s, 2);
            n = get_int(s);
            break;
        case BITDECS:
        case BITCOMS:
        case BITDECC:
            m = get_int(s) - 1;
            get_ints(r, s, 1);
            get_vector(m, start, s);
            break;
        case CONVCINT:
        case CONVCBIT:
            get_ints(r, s, 2);
            break;
        case REVEAL:
        case CONVSINT:
            n = get_int(s);
            get_ints(r, s, 2);
            break;
        case LDMSDI:
        case STMSDI:
        case LDMSD:
        case STMSD:
        case STMSDCI:
        case XORS:
        case ANDRS:
        case ANDS:
        case INPUTB:
            get_vector(get_int(s), start, s);
            break;
        case PRINTREGSIGNED:
            n = get_int(s);
            get_ints(r, s, 1);
            break;
        case TRANS:
            m = get_int(s) - 1;
            n = get_int(s);
            get_vector(m, start, s);
            break;
        default:
            ostringstream os;
            os << "Invalid instruction " << showbase << hex  << opcode
                    << " at " << dec << pos << "/" << hex << file_pos << dec;
            throw Invalid_Instruction(os.str());
        }
    }

    switch(opcode)
    {
#define X(NAME, CODE) case NAME: \
      break;
    INSTRUCTIONS
#undef X
    default:
        ostringstream os;
        os << "Code not defined for instruction " << showbase << hex << opcode << dec;
        os << "This virtual machine executes binary circuits only." << endl;
        os << "Try compiling with '-B' or use only sbit* types." << endl;
        throw Invalid_Instruction(os.str());
        break;
    }
}

} /* namespace GC */
