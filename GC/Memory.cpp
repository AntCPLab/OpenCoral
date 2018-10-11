// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Memory.cpp
 *
 */

#include <GC/Memory.h>

#include "Math/Integer.h"

#include "GC/Clear.h"
#include <GC/FakeSecret.h>
#include "Secret.h"

#include "Yao/YaoGarbleWire.h"
#include "Yao/YaoEvalWire.h"

namespace GC
{

template <class T>
void Memory<T>::resize(size_t size, const char* name)
{
    cout << "Resizing " << T::type_string() << " " << name << " to " << size << endl;
    vector<T>::resize(size);
}

template class Memory<FakeSecret>;
template class Memory<Clear>;
template class Memory<Integer>;
template class Memory< Secret<PRFRegister> >;
template class Memory< Secret<EvalRegister> >;
template class Memory< Secret<GarbleRegister> >;
template class Memory< Secret<RandomRegister> >;
template class Memory< Secret<YaoGarbleWire> >;
template class Memory< Secret<YaoEvalWire> >;
template class Memory< AuthValue >;
template class Memory< SpdzShare >;

} /* namespace GC */
