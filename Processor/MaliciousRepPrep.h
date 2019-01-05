/*
 * MaliciousRepPrep.h
 *
 */

#ifndef PROCESSOR_MALICIOUSREPPREP_H_
#define PROCESSOR_MALICIOUSREPPREP_H_

#include "Data_Files.h"
#include "ReplicatedPrep.h"
#include "Math/MaliciousRep3Share.h"
#include "Auth/MaliciousRepMC.h"

#include <array>

template<class T>
class MaliciousRepPrep : public BufferPrep<T>
{
    typedef BufferPrep<T> super;

    ReplicatedPrep<typename T::Honest> honest_prep;
    typename T::Honest::Protocol* replicated;
    typename T::MAC_Check MC;

    vector<T> masked;
    vector<T> checks;
    vector <typename T::clear> opened;

    vector<array<T, 3>> check_triples;
    vector<array<T, 2>> check_squares;

    void clear_tmp();

    void buffer_triples();
    void buffer_squares();
    void buffer_inverses();
    void buffer_bits();

public:
    MaliciousRepPrep(SubProcessor<T>* proc);
    ~MaliciousRepPrep();

    void set_protocol(Beaver<T>& protocol);
};

#endif /* PROCESSOR_MALICIOUSREPPREP_H_ */
