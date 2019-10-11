/*
 * MaliciousRepPrep.h
 *
 */

#ifndef GC_REPPREP_H_
#define GC_REPPREP_H_

#include "MaliciousRepSecret.h"
#include "ShiftableTripleBuffer.h"
#include "Protocols/ReplicatedPrep.h"

namespace GC
{

template<class T>
class RepPrep : public BufferPrep<T>, ShiftableTripleBuffer<T>
{
    ReplicatedBase* protocol;

public:
    RepPrep(DataPositions& usage, Thread<T>& thread);
    ~RepPrep();

    void set_protocol(typename T::Protocol& protocol);

    void buffer_triples();
    void buffer_bits();

    void buffer_squares() { throw not_implemented(); }
    void buffer_inverses() { throw not_implemented(); }

    void get(Dtype type, T* data)
    {
        BufferPrep<T>::get(type, data);
    }

    array<T, 3> get_triple(int n_bits)
    {
        return ShiftableTripleBuffer<T>::get_triple(n_bits);
    }
};

} /* namespace GC */

#endif /* GC_REPPREP_H_ */
