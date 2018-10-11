// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Program.h
 *
 */

#ifndef GC_PROGRAM_H_
#define GC_PROGRAM_H_

#include "GC/Instruction.h"

#include <vector>
using namespace std;

namespace GC
{

enum BreakType {
    TIME_BREAK,
    DONE_BREAK,
    CAP_BREAK,
};

template <class T> class Processor;

template <class T>
class Program
{
    vector< Instruction<T> > p;
    int offline_data_used;

    // Maximal register used
    int max_reg[MAX_REG_TYPE];

    // Memory size used directly
    int max_mem[MAX_REG_TYPE];

    // True if program contains variable-sized loop
    bool unknown_usage;

    void compute_constants();

    public:

    Program();

    // Read in a program
    void parse(const string& bytecoode_name);
    void parse(istream& s);

    int get_offline_data_used() const { return offline_data_used; }
    void print_offline_cost() const;

    bool usage_unknown() const { return unknown_usage; }

    int num_reg(RegType reg_type) const
      { return max_reg[reg_type]; }

    int direct_mem(RegType reg_type) const
      { return max_mem[reg_type]; }

    // Execute this program, updateing the processor and memory
    // and streams pointing to the triples etc
    BreakType execute(Processor<T>& Proc, int PC = -1) const;

    bool done(Processor<T>& Proc) const { return Proc.PC >= p.size(); }

    template <class U>
    Program<U>& cast() { return *reinterpret_cast< Program<U>* >(this); }
};


} /* namespace GC */

#endif /* GC_PROGRAM_H_ */
