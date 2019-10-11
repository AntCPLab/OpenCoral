/*
 * ReplicatedPrep.h
 *
 */

#ifndef GC_SEMIHONESTREPPREP_H_
#define GC_SEMIHONESTREPPREP_H_

#include "RepPrep.h"
#include "ShareSecret.h"

namespace GC
{

class SemiHonestRepPrep : public RepPrep<SemiHonestRepSecret>
{
public:
    SemiHonestRepPrep(DataPositions& usage, Thread<SemiHonestRepSecret>& thread) :
            RepPrep<SemiHonestRepSecret>(usage, thread)
    {
    }

    void buffer_triples() { throw not_implemented(); }
};

} /* namespace GC */

#endif /* GC_SEMIHONESTREPPREP_H_ */
