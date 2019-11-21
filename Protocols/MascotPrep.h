/*
 * MascotPrep.h
 *
 */

#ifndef PROTOCOLS_MASCOTPREP_H_
#define PROTOCOLS_MASCOTPREP_H_

#include "ReplicatedPrep.h"
#include "RandomPrep.h"
#include "OT/TripleMachine.h"

template<class T>
class OTPrep : public virtual RingPrep<T>
{
public:
    typename T::TripleGenerator* triple_generator;

    MascotParams params;

    OTPrep<T>(SubProcessor<T>* proc, DataPositions& usage);
    ~OTPrep();

    void set_protocol(typename T::Protocol& protocol);

    size_t data_sent();
    NamedCommStats comm_stats();
};

template<class T>
class MascotPrep : public OTPrep<T>, public RandomPrep<T>
{
public:
    MascotPrep(SubProcessor<T>* proc, DataPositions& usage) :
            RingPrep<T>(proc, usage), OTPrep<T>(proc, usage)
    {
    }

    void buffer_triples();
    void buffer_inputs(int player);
    void buffer_bits() { throw runtime_error("use subclass"); }

    T get_random();
};

template<class T>
class MascotFieldPrep : public MascotPrep<T>
{
    void buffer_inverses();
    void buffer_bits();

public:
    MascotFieldPrep<T>(SubProcessor<T>* proc, DataPositions& usage) :
            RingPrep<T>(proc, usage), MascotPrep<T>(proc, usage)
    {
    }
};

#endif /* PROTOCOLS_MASCOTPREP_H_ */
