// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Processor.cpp
 *
 */

#include <GC/Processor.h>

#include <iostream>
using namespace std;

#include "GC/Program.h"
#include "Secret.h"
#include "Access.h"

#include "Yao/YaoGarbleWire.h"
#include "Yao/YaoEvalWire.h"

namespace GC
{

template <class T>
Processor<T>* Processor<T>::singleton = 0;

template <class T>
Processor<T>& Processor<T>::s()
{
	if (singleton == 0)
		throw runtime_error("no singleton");
	return *singleton;
}

template <class T>
Processor<T>::Processor(Machine<T>& machine) :
		machine(machine), PC(0), time(0),
		complexity(0)
{
	if (singleton)
		throw runtime_error("there can only be one");
	singleton = this;
}

template<class T>
Processor<T>::~Processor()
{
	cout << "Finished after " << time << " instructions" << endl;
}

template <class T>
void Processor<T>::reset(const Program<T>& program)
{
    S.resize(program.num_reg(SBIT), "registers");
    C.resize(program.num_reg(CBIT), "registers");
    I.resize(program.num_reg(INT), "registers");
    machine.reset(program);
}

template<class T>
void GC::Processor<T>::open_input_file(const string& name)
{
    cout << "opening " << name << endl;
    input_file.open(name);
    input_file.exceptions(ios::badbit | ios::eofbit);
}

template <class T>
void Processor<T>::bitdecc(const vector<int>& regs, const Clear& x)
{
    for (unsigned int i = 0; i < regs.size(); i++)
        C[regs[i]] = (x >> i) & 1;
}

template <class T>
void Processor<T>::bitdecint(const vector<int>& regs, const Integer& x)
{
    for (unsigned int i = 0; i < regs.size(); i++)
        I[regs[i]] = (x >> i) & 1;
}

template<class T>
void GC::Processor<T>::load_dynamic_direct(const vector<int>& args)
{
    vector< ReadAccess<T> > accesses;
    if (args.size() % 3 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 3)
        accesses.push_back({S[args[i]], args[i+1], args[i+2], complexity});
    T::load(accesses, machine.MD);
}

template<class T>
void GC::Processor<T>::load_dynamic_indirect(const vector<int>& args)
{
    vector< ReadAccess<T> > accesses;
    if (args.size() % 3 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 3)
        accesses.push_back({S[args[i]], C[args[i+1]], args[i+2], complexity});
    T::load(accesses, machine.MD);
}

template<class T>
void GC::Processor<T>::store_dynamic_direct(const vector<int>& args)
{
    vector< WriteAccess<T> > accesses;
    if (args.size() % 2 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 2)
        accesses.push_back({args[i+1], S[args[i]]});
    T::store(machine.MD, accesses);
    complexity += accesses.size() / 2 * T::default_length;
}

template<class T>
void GC::Processor<T>::store_dynamic_indirect(const vector<int>& args)
{
    vector< WriteAccess<T> > accesses;
    if (args.size() % 2 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 2)
        accesses.push_back({C[args[i+1]], S[args[i]]});
    T::store(machine.MD, accesses);
    complexity += accesses.size() / 2 * T::default_length;
}

template <class T>
int GC::Processor<T>::check_args(const vector<int>& args, int n)
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

template<class T>
void GC::Processor<T>::store_clear_in_dynamic(const vector<int>& args)
{
    vector<ClearWriteAccess> accesses;
	check_args(args, 2);
    for (size_t i = 0; i < args.size(); i += 2)
    	accesses.push_back({C[args[i+1]], C[args[i]]});
    T::store_clear_in_dynamic(machine.MD, accesses);
}

template <class T>
void Processor<T>::xors(const vector<int>& args)
{
    check_args(args, 4);
    size_t n_args = args.size();
    for (size_t i = 0; i < n_args; i += 4)
    {
        S[args[i+1]].xor_(args[i], S[args[i+2]], S[args[i+3]]);
#ifndef FREE_XOR
        complexity += args[i];
#endif
    }
}

template <class T>
void Processor<T>::andrs(const vector<int>& args)
{
    check_args(args, 4);
    for (size_t i = 0; i < args.size(); i += 4)
    {
        S[args[i+1]].andrs(args[i], S[args[i+2]], S[args[i+3]]);
        complexity += args[i];
    }
}

template <class T>
void Processor<T>::input(const vector<int>& args)
{
    check_args(args, 3);
    for (size_t i = 0; i < args.size(); i += 3)
    {
        S[args[i+2]] = T::input(args[i] + 1, input_file, args[i+1]);
#ifdef DEBUG_INPUT
        cout << "input to " << args[i+2] << "/" << &S[args[i+2]] << endl;
#endif
    }
}

template <class T>
void Processor<T>::print_reg(int reg, int n)
{
#ifdef DEBUG_VALUES
    cout << "print_reg " << typeid(T).name() << " " << reg << " " << &C[reg] << endl;
#endif
    T::out << "Reg[" << reg << "] = " << hex << showbase << C[reg] << dec << " # ";
    print_str(n);
    T::out << endl << flush;
}

template <class T>
void Processor<T>::print_reg_plain(Clear& value)
{
    T::out << hex << showbase << value << dec << flush;
}

template <class T>
void Processor<T>::print_reg_signed(unsigned n_bits, Clear& value)
{
    unsigned n_shift = 0;
    if (n_bits > 1)
        n_shift = sizeof(value.get()) * 8 - n_bits;
    T::out << dec << (value.get() << n_shift >> n_shift) << flush;
}

template <class T>
void Processor<T>::print_chr(int n)
{
    T::out << (char)n << flush;
}

template <class T>
void Processor<T>::print_str(int n)
{
    T::out << string((char*)&n,sizeof(n)) << flush;
}

template class Processor<FakeSecret>;
template class Processor< Secret<PRFRegister> >;
template class Processor< Secret<EvalRegister> >;
template class Processor< Secret<GarbleRegister> >;
template class Processor< Secret<RandomRegister> >;
template class Processor< Secret<YaoGarbleWire> >;
template class Processor< Secret<YaoEvalWire> >;

} /* namespace GC */
