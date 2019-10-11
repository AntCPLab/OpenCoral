/*
 * TinyPrep.h
 *
 */

#ifndef GC_TINYPREP_H_
#define GC_TINYPREP_H_

#include "Thread.h"
#include "OT/TripleMachine.h"
#include "Protocols/Beaver.h"
#include "Protocols/ReplicatedPrep.h"
#include "Protocols/RandomPrep.h"

namespace GC
{

template<class T>
class TinyPrep : public BufferPrep<T>, public RandomPrep<typename T::part_type::super>
{
    typedef Share<Z2<1 + T::part_type::s>> res_type;

    Thread<T>& thread;

    typename T::TripleGenerator* triple_generator;
    typename T::part_type::TripleGenerator* input_generator;
    MascotParams params;

    vector<array<typename T::part_type, 3>> triple_buffer;

public:
    TinyPrep(DataPositions& usage, Thread<T>& thread);
    ~TinyPrep();

    void set_protocol(Beaver<T>& protocol);

    void buffer_triples();
    void buffer_bits();

    void buffer_inputs(int player);

    void buffer_squares() { throw not_implemented(); }
    void buffer_inverses() { throw not_implemented(); }

    typename T::part_type::super get_random();

    array<T, 3> get_triple(int n_bits);
};

} /* namespace GC */

#endif /* GC_TINYPREP_H_ */
