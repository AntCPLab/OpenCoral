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

inline
Instruction::Instruction() :
        BaseInstruction()
{
    size = 1;
}

inline
bool Instruction::get_offline_data_usage(int& usage)
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

inline
unsigned Instruction::get_mem(RegType reg_type) const
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
    default:
        return BaseInstruction::get_mem(reg_type, MAX_SECRECY_TYPE);
    }

    return 0;
}

inline
void Instruction::parse(istream& s, int pos)
{
    n = 0;
    start.resize(0);
    ::memset(r, 0, sizeof(r));

    int file_pos = s.tellg();
    opcode = ::get_int(s);

    parse_operands(s, pos, file_pos);

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
