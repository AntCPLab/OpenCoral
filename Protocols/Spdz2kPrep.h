/*
 * Spdz2kPrep.h
 *
 */

#ifndef PROTOCOLS_SPDZ2KPREP_H_
#define PROTOCOLS_SPDZ2KPREP_H_

#include "MascotPrep.h"
#include "Spdz2kShare.h"

template<class T, class U>
void bits_from_square_in_ring(vector<T>& bits, int buffer_size, U* bit_prep);

template<class T>
class Spdz2kPrep : public MascotPrep<T>
{
    typedef Spdz2kShare<T::k + 2, T::s> BitShare;
    DataPositions bit_pos;
    MascotPrep<BitShare>* bit_prep;
    SubProcessor<BitShare>* bit_proc;
    typename BitShare::MAC_Check* bit_MC;
    Sub_Data_Files<BitShare>* bit_DataF;
    typename BitShare::Protocol* bit_protocol;

public:
    Spdz2kPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~Spdz2kPrep();

    void set_protocol(typename T::Protocol& protocol);

    void buffer_inverses() { throw division_by_zero(); }
    void buffer_bits();

    size_t data_sent();
};

#endif /* PROTOCOLS_SPDZ2KPREP_H_ */
