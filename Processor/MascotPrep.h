/*
 * MascotPrep.h
 *
 */

#ifndef PROCESSOR_MASCOTPREP_H_
#define PROCESSOR_MASCOTPREP_H_

#include "ReplicatedPrep.h"
#include "OT/NPartyTripleGenerator.h"

template<class T>
class OTPrep : public RingPrep<T>
{
protected:
    typename T::TripleGenerator* triple_generator;

public:
    MascotParams params;

    OTPrep<T>(SubProcessor<T>* proc, DataPositions& usage);
    ~OTPrep();

    void set_protocol(typename T::Protocol& protocol);

    size_t data_sent();
};

template<class T>
class MascotPrep : public OTPrep<T>
{
public:
    MascotPrep(SubProcessor<T>* proc, DataPositions& usage) : OTPrep<T>(proc, usage)
    {
    }

    void buffer_triples();
    void buffer_inputs(int player);

    T get_random();
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
