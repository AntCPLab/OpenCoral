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
#include "Protocols/RmfeShareConverter.h"
#include "TinyOT/tinyotshare.h"
#include "TinyOT/tinyotinput.h"


namespace GC
{

// typedef TinySecret<DEFAULT_SECURITY> Spdz2kBShare;

template<class T>
class RmfeSharePrep : public PersonalPrep<T> {
    const int s = 40;

    typename T::TripleGenerator* triple_generator;
    // typename T::whole_type::TripleGenerator* real_triple_generator;
    MascotParams params;

    RmfeShareConverter<TinyOTShare>* tinyot2rmfe;

    // Put it as an instance variable instead of function local variable to save some RTTs.
    GlobalPRNG* shared_prng;

    typedef typename T::whole_type secret_type;

    // void init_real(Player& P);
    
    using PersonalPrep<T>::input_player;
    using Preprocessing<T>::count;
    using Preprocessing<T>::waste;
    using Preprocessing<T>::do_count;
    using BufferPrep<T>::quintuples;

public:
    RmfeSharePrep(DataPositions& usage, int input_player = PersonalPrep<T>::SECURE);
    RmfeSharePrep(SubProcessor<T>*, DataPositions& usage);
    ~RmfeSharePrep();

    PRNG prng;
    Player* P;
    typename T::mac_key_type::Scalar revealed_key;

    const int NORMAL_SACRIFICE = 40;

    void buffer_triples();
    // void buffer_squares() { throw not_implemented(); }
    // void buffer_bits();
    // void buffer_inverses() { throw not_implemented(); }
    void buffer_inputs(int player);

    void buffer_personal_triples(size_t n, ThreadQueues* queues = 0) { throw runtime_error("no personal triples"); }
    void buffer_personal_triples(vector<array<T, 3>>& quintuples, size_t begin, size_t end) { throw runtime_error("no personal triples"); }

    void buffer_personal_quintuples(size_t n, ThreadQueues* queues = 0);
    void buffer_personal_quintuples(vector<array<T, 5>>& quintuples, size_t begin, size_t end);

    void buffer_normals();

    // virtual array<T, 5> get_quintuple(int n_bits);
    // virtual array<T, 5> get_quintuple_no_count(int n_bits);

    void set_protocol(typename T::Protocol& protocol);
    // void set_mc(typename T::MAC_Check* MC);

    Preprocessing<typename T::part_type>& get_part()
    {
        return *this;
    }

#ifdef INSECURE_RMFE_PREP
//// Debug APIs
    void get_input_no_count(T& r_share, typename T::open_type& r , int player);
    void get_three_no_count(Dtype dtype, T& a, T& b, T& c);
#endif
};

}

#endif /* GC_RMFESHAREPREP_H_ */
