/*
 * SemiPrep2k.h
 *
 */

#ifndef PROTOCOLS_SEMIPREP2K_H_
#define PROTOCOLS_SEMIPREP2K_H_

#include "SemiPrep.h"

template<class T>
class SemiPrep2k : public SemiPrep<T>
{
public:
    SemiPrep2k(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage),
            OTPrep<T>(proc, usage), SemiHonestRingPrep<T>(proc, usage),
            SemiPrep<T>(proc, usage)
    {
    }

    void get_dabit_no_count(T& a, typename T::bit_type& b)
    {
        this->get_one_no_count(DATA_BIT, a);
        b = a & 1;
    }
};

#endif /* PROTOCOLS_SEMIPREP2K_H_ */
