/*
 * Processor.h
 *
 */

#ifndef GC_PROCESSOR_H_
#define GC_PROCESSOR_H_

#include <vector>
using namespace std;

#include "GC/Clear.h"
#include "GC/Machine.h"

#include "Math/Integer.h"
#include "Processor/ProcessorBase.h"

namespace GC
{

template <class T> class Program;

class ExecutionStats : public map<int, size_t>
{
public:
    ExecutionStats& operator+=(const ExecutionStats& other)
    {
        for (auto it : other)
            (*this)[it.first] += it.second;
        return *this;
    }
};

template <class T>
class Processor : public ::ProcessorBase
{
public:
    static int check_args(const vector<int>& args, int n);

    static void check_input(long long in, int n_bits);

    Machine<T>& machine;

    unsigned int PC;
    unsigned int time;

    // rough measure for the memory usage
    size_t complexity;

    Memory<T> S;
    Memory<Clear> C;
    Memory<Integer> I;

    ExecutionStats stats;

    Processor(Machine<T>& machine);
    ~Processor();

    void reset(const Program<T>& program, int arg);
    void reset(const Program<T>& program);

    long long get_input(int n_bits, bool interactive = false);

    void bitcoms(T& x, const vector<int>& regs) { x.bitcom(S, regs); }
    void bitdecs(const vector<int>& regs, const T& x) { x.bitdec(S, regs); }
    void bitdecc(const vector<int>& regs, const Clear& x);
    void bitdecint(const vector<int>& regs, const Integer& x);

    void random_bit(T &x) { x.random_bit(); }

    void load_dynamic_direct(const vector<int>& args);
    void store_dynamic_direct(const vector<int>& args);
    void load_dynamic_indirect(const vector<int>& args);
    void store_dynamic_indirect(const vector<int>& args);
    void store_clear_in_dynamic(const vector<int>& args);

    void xors(const vector<int>& args);
    void and_(const vector<int>& args, bool repeat);
    void andrs(const vector<int>& args) { and_(args, true); }
    void ands(const vector<int>& args) { and_(args, false); }

    void input(const vector<int>& args);

    void print_reg(int reg, int n);
    void print_reg_plain(Clear& value);
    void print_reg_signed(unsigned n_bits, Clear& value);
    void print_chr(int n);
    void print_str(int n);
    void print_float(const vector<int>& args);
    void print_float_prec(int n);
};

template <class T>
inline int GC::Processor<T>::check_args(const vector<int>& args, int n)
{
    if (args.size() % n != 0)
        throw runtime_error("invalid number of arguments");
    int total = 0;
    for (size_t i = 0; i < args.size(); i += n)
    {
        total += args[i];
    }
    return total;
}

} /* namespace GC */

#endif /* GC_PROCESSOR_H_ */
