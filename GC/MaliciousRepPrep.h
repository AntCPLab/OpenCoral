/*
 * MaliciousRepPrep.h
 *
 */

#ifndef GC_MALICIOUSREPPREP_H_
#define GC_MALICIOUSREPPREP_H_

#include "MaliciousRepSecret.h"
#include "Protocols/ReplicatedPrep.h"

namespace GC
{

class MaliciousRepPrep : public BufferPrep<MaliciousRepSecret>
{
    ReplicatedBase* protocol;

public:
    MaliciousRepPrep(DataPositions& usage);
    ~MaliciousRepPrep();

    void set_protocol(MaliciousRepSecret::Protocol& protocol);

    void buffer_triples();
    void buffer_bits();

    void buffer_squares() { throw not_implemented(); }
    void buffer_inverses() { throw not_implemented(); }
};

} /* namespace GC */

#endif /* GC_MALICIOUSREPPREP_H_ */
