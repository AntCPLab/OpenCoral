// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Machine.cpp
 *
 */

#include <GC/Machine.h>

#include "GC/Program.h"
#include "Secret.h"

#include "Yao/YaoGarbleWire.h"
#include "Yao/YaoEvalWire.h"

namespace GC
{

template <class T>
Machine<T>::Machine(Memory<typename T::DynamicType>& dynamic_memory) : MD(dynamic_memory)
{
    start_timer();
}

template<class T>
Machine<T>::~Machine()
{
    for (auto it = timer.begin(); it != timer.end(); it++)
        cout << T::phase_name() << " timer " << it->first << " at end: "
                << it->second.elapsed() << " seconds" << endl;
}

template <class T>
void Machine<T>::reset(const Program<T>& program)
{
    MS.resize(program.direct_mem(SBIT), "memory");
    MC.resize(program.direct_mem(CBIT), "memory");
    MI.resize(program.direct_mem(INT), "memory");
    MD.resize(program.direct_mem(DYN_SBIT), "dynamic memory");
}

template class Machine<FakeSecret>;
template class Machine< Secret<PRFRegister> >;
template class Machine< Secret<EvalRegister> >;
template class Machine< Secret<GarbleRegister> >;
template class Machine< Secret<RandomRegister> >;
template class Machine< Secret<YaoGarbleWire> >;
template class Machine< Secret<YaoEvalWire> >;

} /* namespace GC */
