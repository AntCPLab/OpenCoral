/*
 * MascotPrep.h
 *
 */

#ifndef PROCESSOR_MASCOTPREP_H_
#define PROCESSOR_MASCOTPREP_H_

#include "ReplicatedPrep.h"
#include "OT/NPartyTripleGenerator.h"

template<class T>
class MascotPrep : public RingPrep<T>
{
protected:
    NPartyTripleGenerator<typename T::prep_type>* triple_generator;

public:
    MascotParams params;

    MascotPrep<T>(SubProcessor<T>* proc, DataPositions& usage);
    ~MascotPrep();

    void set_protocol(typename T::Protocol& protocol);

    void buffer_triples();
    void buffer_inputs(int player);

    T get_random();

    size_t data_sent();
};

template<class T>
class MascotFieldPrep : public MascotPrep<T>
{
    void buffer_inverses();
    void buffer_bits();

public:
    MascotFieldPrep<T>(SubProcessor<T>* proc, DataPositions& usage) :
            MascotPrep<T>(proc, usage) {}
};

#endif /* PROCESSOR_MASCOTPREP_H_ */
