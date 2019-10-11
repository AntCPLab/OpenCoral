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
class MaliciousRepPrep : public BufferPrep<T>
{
    template<class U> friend class MalRepRingPrep;

    typedef BufferPrep<T> super;

    DataPositions honest_usage;
    ReplicatedPrep<typename T::Honest> honest_prep;
    typename T::Honest::Protocol* replicated;
    typename T::MAC_Check MC;
    SubProcessor<T>* proc;

    vector<T> masked;
    vector<T> checks;
    vector <typename T::clear> opened;

    vector<array<T, 5>> check_triples;
    vector<array<T, 2>> check_squares;

    void clear_tmp();

    void buffer_triples();
    void buffer_squares();
    void buffer_inverses();
    void buffer_bits();
    void buffer_inputs(int player);

public:
    MaliciousRepPrep(SubProcessor<T>* proc, DataPositions& usage);
    MaliciousRepPrep(DataPositions& usage);
    ~MaliciousRepPrep();

    void set_protocol(typename T::Protocol& protocol);
    void init_honest(Player& P);
};

#endif /* PROTOCOLS_MALICIOUSREPPREP_H_ */
