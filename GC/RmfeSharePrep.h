/*
 * RmfeSharePrep.h
 *
 */

#ifndef GC_RMFESHAREPREP_H_
#define GC_RMFESHAREPREP_H_

#include "Protocols/ReplicatedPrep.h"
#include "OT/NPartyTripleGenerator.h"
#include "ShareThread.h"
#include "PersonalPrep.h"

namespace GC
{

template<class T>
class RmfeSharePrep : public PersonalPrep<T>
{
    typename T::TripleGenerator* triple_generator;
    typename T::whole_type::TripleGenerator* real_triple_generator;
    MascotParams params;

    typedef typename T::whole_type secret_type;

    void buffer_triples();
    void buffer_squares() { throw not_implemented(); }
    void buffer_bits();
    void buffer_inverses() { throw not_implemented(); }

    void buffer_inputs(int player);

    void buffer_secret_triples();

    void init_real(Player& P);

public:
    RmfeSharePrep(DataPositions& usage, int input_player =
            PersonalPrep<T>::SECURE);
    RmfeSharePrep(SubProcessor<T>*, DataPositions& usage);
    ~RmfeSharePrep();

    void set_protocol(typename T::Protocol& protocol);
};

}

#endif /* GC_RMFESHAREPREP_H_ */
