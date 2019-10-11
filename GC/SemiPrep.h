/*
 * SemiPrep.h
 *
 */

#ifndef GC_SEMIPREP_H_
#define GC_SEMIPREP_H_

#include "Protocols/ReplicatedPrep.h"
#include "OT/TripleMachine.h"
#include "SemiSecret.h"
#include "ShiftableTripleBuffer.h"

template<class T> class Beaver;

namespace GC
{

class SemiPrep : public BufferPrep<SemiSecret>, ShiftableTripleBuffer<SemiSecret>
{
    Thread<SemiSecret>& thread;

    SemiSecret::TripleGenerator* triple_generator;
    MascotParams params;

public:
    SemiPrep(DataPositions& usage, Thread<SemiSecret>& thread);
    ~SemiPrep();

    void set_protocol(Beaver<SemiSecret>& protocol);

    void buffer_triples();
    void buffer_bits();

    void buffer_squares() { throw not_implemented(); }
    void buffer_inverses() { throw not_implemented(); }

    void get(Dtype type, SemiSecret* data)
    {
        BufferPrep<SemiSecret>::get(type, data);
    }

    array<SemiSecret, 3> get_triple(int n_bits)
    {
        return ShiftableTripleBuffer<SemiSecret>::get_triple(n_bits);
    }
};

} /* namespace GC */

#endif /* GC_SEMIPREP_H_ */
