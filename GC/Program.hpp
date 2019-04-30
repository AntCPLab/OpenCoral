/*
 * Program.cpp
 *
 */

#include <GC/Program.h>

#include "config.h"

#include "Tools/callgrind.h"

namespace GC
{

template <class T>
Program<T>::Program() :
        offline_data_used(0), unknown_usage(false)
{
    compute_constants();
}

template <class T>
void Program<T>::compute_constants()
{
    for (int reg_type = 0; reg_type < MAX_REG_TYPE; reg_type++)
    {
        max_reg[reg_type] = 0;
        max_mem[reg_type] = 0;
    }
    for (unsigned int i = 0; i < p.size(); i++)
    {
        if (!p[i].get_offline_data_usage(offline_data_used))
            unknown_usage = true;
        for (int reg_type = 0; reg_type < MAX_REG_TYPE; reg_type++)
        {
            max_reg[reg_type] = max(max_reg[reg_type],
                    p[i].get_max_reg(RegType(reg_type)));
            max_mem[reg_type] = max(max_mem[reg_type],
                    p[i].get_mem(RegType(reg_type)));
        }
    }
}

template <class T>
void Program<T>::parse(const string& bytecode_name)
{
    string filename = "Programs/Bytecode/" + bytecode_name + ".bc";
    parse_file(filename);
}

template <class T>
void Program<T>::parse_file(const string& filename)
{
    ifstream s(filename.c_str());
    if (s.bad() or s.fail())
        throw runtime_error("Cannot open " + filename);
    parse(s);
}

template <class T>
void Program<T>::parse(istream& s)
{
    p.resize(0);
    Instruction<T> instr;
    s.peek();
    int pos = 0;
    CALLGRIND_STOP_INSTRUMENTATION;
    while (!s.eof())
    {
        if (s.bad() or s.fail())
            throw runtime_error("error reading program");
        instr.parse(s, pos);
        p.push_back(instr);
        //cerr << "\t" << instr << endl;
        s.peek();
        pos++;
    }
    CALLGRIND_START_INSTRUMENTATION;
    compute_constants();
}

template <class T>
void Program<T>::print_offline_cost() const
{
    if (unknown_usage)
    {
        cerr << "Tape has unknown usage" << endl;
        return;
    }

    cerr << "Cost of first tape: " << offline_data_used << endl;
}

template <class T>
template <class U>
__attribute__((flatten))
BreakType Program<T>::execute(Processor<T>& Proc, U& dynamic_memory,
        int PC) const
{
    if (PC != -1)
        Proc.PC = PC;
#ifdef DEBUG_ROUNDS
    cout << typeid(T).name() << " starting at PC " << Proc.PC << endl;
#endif
    unsigned int size = p.size();
    size_t time = Proc.time;
    Proc.complexity = 0;
    do
    {
#ifdef DEBUG_EXE
    	cout << "execute " << time << "/" << Proc.PC << endl;
#endif
        if (Proc.PC >= size)
        {
            Proc.time = time;
            return DONE_BREAK;
        }
#ifdef COUNT_INSTRUCTIONS
        Proc.stats[p[Proc.PC].get_opcode()]++;
#endif
        p[Proc.PC++].execute(Proc, dynamic_memory);
        time++;
#ifdef DEBUG_COMPLEXITY
        cout << "complexity at " << time << ": " << Proc.complexity << endl;
#endif
    }
    while (Proc.complexity < (1 << 19));
    Proc.time = time;
#ifdef DEBUG_ROUNDS
    cout << "breaking at time " << Proc.time << endl;
#endif
    return TIME_BREAK;
}

} /* namespace GC */
