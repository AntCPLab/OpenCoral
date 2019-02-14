/*
 * Replicated.h
 *
 */

#ifndef PROCESSOR_REPLICATED_H_
#define PROCESSOR_REPLICATED_H_

#include <assert.h>
#include <vector>
using namespace std;

#include "Tools/octetStream.h"
#include "Tools/random.h"
#include "Networking/Player.h"

template<class T> class SubProcessor;
template<class T> class ReplicatedMC;
template<class T> class ReplicatedInput;
template<class T> class ReplicatedPrivateOutput;
template<class T> class Share;
template<class T> class Rep3Share;
template<class T> class MAC_Check_Base;

class ReplicatedBase
{
public:
    PRNG shared_prngs[2];

    Player& P;

    ReplicatedBase(Player& P);

    int get_n_relevant_players() { return P.num_players() - 1; }
};

template <class T>
class ProtocolBase
{
public:
    int counter;

    ProtocolBase();
    virtual ~ProtocolBase();

    void muls(const vector<int>& reg, SubProcessor<T>& proc, MAC_Check_Base<T>& MC,
            int size);
    void mulrs(const vector<int>& reg, SubProcessor<T>& proc);
    void dotprods(const vector<int>& reg, SubProcessor<T>& proc);

    virtual void init_mul(SubProcessor<T>* proc) = 0;
    virtual typename T::clear prepare_mul(const T& x, const T& y) = 0;
    virtual void exchange() = 0;
    virtual T finalize_mul() = 0;

    void init_dotprod(SubProcessor<T>* proc) { init_mul(proc); }
    void prepare_dotprod(const T& x, const T& y) { prepare_mul(x, y); }
    void next_dotprod() {}
    T finalize_dotprod(int length);
};

template <class T>
class Replicated : public ReplicatedBase, public ProtocolBase<T>
{
    vector<octetStream> os;
    deque<typename T::clear> add_shares;
    typename T::clear dotprod_share;

public:
    typedef ReplicatedMC<T> MAC_Check;
    typedef ReplicatedInput<T> Input;
    typedef ReplicatedPrivateOutput<T> PrivateOutput;

    Replicated(Player& P);

    static void assign(T& share, const typename T::clear& value, int my_num)
    {
        assert(T::length == 2);
        share.assign_zero();
        if (my_num < 2)
            share[my_num] = value;
    }

    void init_mul(SubProcessor<T>* proc);
    void init_mul();
    typename T::clear prepare_mul(const T& x, const T& y);
    void exchange();
    T finalize_mul();

    void prepare_reshare(const typename T::clear& share);

    void init_dotprod(SubProcessor<T>* proc);
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    T get_random();
};

#endif /* PROCESSOR_REPLICATED_H_ */
