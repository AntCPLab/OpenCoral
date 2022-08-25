/*
 * SemiPrep.cpp
 *
 */

#include "SemiPrep.h"

#include "ReplicatedPrep.hpp"

template<class T>
SemiPrep<T>::SemiPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage),
        BitPrep<T>(proc, usage),
        OTPrep<T>(proc, usage),
        RingPrep<T>(proc, usage),
        SemiHonestRingPrep<T>(proc, usage)
{
    this->params.set_passive();
}

template<class T>
void SemiPrep<T>::buffer_triples()
{
    assert(this->triple_generator);
    this->triple_generator->generatePlainTriples();
    for (auto& x : this->triple_generator->plainTriples)
    {
        this->triples.push_back({{x[0], x[1], x[2]}});
    }
    this->triple_generator->unlock();
}

template<class T>
void SemiPrep<T>::buffer_bits()
{
    assert(this->proc);
    if (this->proc->P.num_players() == 2 and not T::clear::characteristic_two)
    {
        assert(this->triple_generator);
        this->triple_generator->generatePlainBits();
        for (auto& x : this->triple_generator->plainBits)
            this->bits.push_back(x);
    }
    else
        SemiHonestRingPrep<T>::buffer_bits();
}
