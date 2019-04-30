/*
 * SemiPrep.h
 *
 */

#ifndef PROCESSOR_SEMIPREP_H_
#define PROCESSOR_SEMIPREP_H_

#include "MascotPrep.h"

template<class T>
class SemiPrep : public OTPrep<T>
{
public:
    SemiPrep(SubProcessor<T>* proc, DataPositions& usage);

    void buffer_triples();
    void buffer_inverses();
};

#endif /* PROCESSOR_SEMIPREP_H_ */
