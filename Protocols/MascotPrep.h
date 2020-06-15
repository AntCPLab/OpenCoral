/*
 * MascotPrep.h
 *
 */

#ifndef PROTOCOLS_MASCOTPREP_H_
#define PROTOCOLS_MASCOTPREP_H_

#include "ReplicatedPrep.h"
#include "RandomPrep.h"
#include "OT/MascotParams.h"

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
class MascotPrep: public virtual MaliciousRingPrep<T>,
        public virtual OTPrep<T>,
        public RandomPrep<T>
{
public:
    MascotPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage),
            MaliciousRingPrep<T>(proc, usage),
            OTPrep<T>(proc, usage)
    {
    }
    virtual ~MascotPrep()
    {
    }

    void buffer_triples();
    void buffer_inputs(int player);
    void buffer_bits() { throw runtime_error("use subclass"); }
    virtual void buffer_dabits(ThreadQueues* queues);
    void buffer_edabits(bool strict, int n_bits, ThreadQueues* queues);

    T get_random();
};

template<class T>
class MascotFieldPrep : public MascotPrep<T>
{
    void buffer_inverses();
    void buffer_bits();

public:
    MascotFieldPrep<T>(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage),
            BitPrep<T>(proc, usage), RingPrep<T>(proc, usage),
            MaliciousRingPrep<T>(proc, usage),
            OTPrep<T>(proc, usage), MascotPrep<T>(proc, usage)
    {
    }
};

#endif /* PROTOCOLS_MASCOTPREP_H_ */
