/*
 * MaliciousRepPrep.h
 *
 */

#ifndef PROTOCOLS_MALICIOUSREPPREP_H_
#define PROTOCOLS_MALICIOUSREPPREP_H_

#include "Processor/Data_Files.h"
#include "ReplicatedPrep.h"
#include "MaliciousRepMC.h"

#include <array>

template<class T> class MalRepRingPrep;
template<int K, int S> class MalRepRingShare;
template<int K, int S> class PostSacriRepRingShare;

template<class T, class U>
void sacrifice(const vector<T>& check_triples, Player& P);

template<class T>
class MaliciousRepPrep : public virtual BufferPrep<T>
{
    template<class U> friend class MalRepRingPrep;

    typedef BufferPrep<T> super;

    DataPositions honest_usage;
    ReplicatedRingPrep<typename T::Honest> honest_prep;
    typename T::Honest::MAC_Check honest_mc;
    SubProcessor<typename T::Honest>* honest_proc;
    typename T::MAC_Check MC;

    vector<T> masked;
    vector<T> checks;
    vector <typename T::open_type> opened;

    vector<array<T, 5>> check_triples;
    vector<array<T, 2>> check_squares;

    void clear_tmp();

protected:
    void buffer_triples();
    void buffer_squares();
    void buffer_inverses();
    void buffer_bits();
    void buffer_inputs(int player);

public:
    MaliciousRepPrep(SubProcessor<T>* proc, DataPositions& usage);
    MaliciousRepPrep(DataPositions& usage, int = 0);
    ~MaliciousRepPrep();

    void set_protocol(typename T::Protocol& protocol);
    void init_honest(Player& P);
};

template<class T>
class MaliciousRepPrepWithBits: public virtual MaliciousRepPrep<T>,
        public virtual MaliciousRingPrep<T>
{
    void buffer_squares()
    {
        MaliciousRepPrep<T>::buffer_squares();
    }

    void buffer_bits()
    {
        MaliciousRepPrep<T>::buffer_bits();
    }

public:
    MaliciousRepPrepWithBits(SubProcessor<T>* proc, DataPositions& usage);

    void set_protocol(typename T::Protocol& protocol)
    {
        MaliciousRingPrep<T>::set_protocol(protocol);
        MaliciousRepPrep<T>::set_protocol(protocol);
    }

};

#endif /* PROTOCOLS_MALICIOUSREPPREP_H_ */
