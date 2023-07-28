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
    // typename T::whole_type::TripleGenerator* real_triple_generator;
    MascotParams params;

    typedef typename T::whole_type secret_type;

    // void init_real(Player& P);

public:
    RmfeSharePrep(DataPositions& usage, int input_player = PersonalPrep<T>::SECURE);
    RmfeSharePrep(SubProcessor<T>*, DataPositions& usage);
    ~RmfeSharePrep();

//// Debug APIs

    typename T::MAC_Check* MC;
    PRNG prng;
    Player* P;
    typename T::mac_key_type::Scalar revealed_key;

    // void buffer_triples();
    // void buffer_squares() { throw not_implemented(); }
    // void buffer_bits();
    // void buffer_inverses() { throw not_implemented(); }
    void buffer_inputs(int player);

    // void get_input_no_count(T& r_share, typename T::open_type& r , int player);
    void get_three_no_count(Dtype dtype, T& a, T& b, T& c);

    void set_protocol(typename T::Protocol& protocol);
    void set_mc(typename T::MAC_Check* MC);
};

}

#endif /* GC_RMFESHAREPREP_H_ */
