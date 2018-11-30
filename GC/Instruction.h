/*
 * Instruction.h
 *
 */

#ifndef PROCESSOR_GC_INSTRUCTION_H_
#define PROCESSOR_GC_INSTRUCTION_H_

#include <vector>
#include <iostream>
using namespace std;

#include "Processor/Instruction.h"

namespace GC
{

// Register types
enum RegType {
    SBIT,
    CBIT,
    INT,
    DYN_SBIT,
    MAX_REG_TYPE,
    NONE
};

template<class T> class Processor;

template <class T>
class Instruction : public ::BaseInstruction
{
    bool (*code)(const Instruction<T>& instruction, Processor<T>& processor);
public:
    Instruction();
    
    int get_r(int i) const { return r[i]; }
    unsigned int get_n() const { return n; }
    const vector<int>& get_start() const { return start; }
    int get_opcode() const { return opcode; }

    // Reads a single instruction from the istream
    void parse(istream& s, int pos);

    // Return whether usage is known
    bool get_offline_data_usage(int& usage);

    int get_reg_type() const;

    // Returns the maximal register used
    unsigned get_max_reg(int reg_type) const;

    // Returns the memory size used if applicable and known
    unsigned get_mem(RegType reg_type) const;

    // Execute this instruction
    bool exe(Processor<T>& processor) const { return code(*this, processor); }
    bool execute(Processor<T>& processor) const;
};

enum
{
    // GC specific
    // write to secret
    SECRET_WRITE = 0x200,
    XORS = 0x200,
    XORM = 0x201,
    ANDRS = 0x202,
    BITDECS = 0x203,
    BITCOMS = 0x204,
    CONVSINT = 0x205,
    LDMSDI = 0x206,
    STMSDI = 0x207,
    LDMSD = 0x208,
    STMSD = 0x209,
    LDBITS = 0x20a,
    ANDS = 0x20b,
    TRANS = 0x20c,
    // write to clear
    CLEAR_WRITE = 0x210,
    XORCI = 0x210,
    BITDECC = 0x211,
    CONVCINT = 0x213,
    REVEAL = 0x214,
    STMSDCI = 0x215,
    INPUTB = 0x216,
    // don't write
    PRINTREGSIGNED = 0x220,
};

} /* namespace GC */

#endif /* PROCESSOR_GC_INSTRUCTION_H_ */
