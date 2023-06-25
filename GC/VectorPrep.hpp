/*
 * VectorPrep.hpp
 *
 */

#ifndef GC_VECTORPREP_HPP_
#define GC_VECTORPREP_HPP_

#include "VectorPrep.h"

#include "Processor/Processor.hpp"

namespace GC
{

template<class T>
VectorPrep<T>::~VectorPrep()
{
    if (part_proc)
        delete part_proc;
}

template<class T>
void VectorPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    auto& thread = ShareThread<T>::s();
    assert(thread.MC);

    if (part_proc)
    {
        assert(&part_proc->MC == &thread.MC->get_part_MC());
        assert(&part_proc->P == &protocol.get_part().P);
        return;
    }

    part_proc = new SubProcessor<typename T::part_type>(
            thread.MC->get_part_MC(), part_prep, protocol.get_part().P);
}

}

#endif /* GC_VECTORPREP_HPP_ */
