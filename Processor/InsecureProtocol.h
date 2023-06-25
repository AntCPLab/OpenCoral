/*
 * InsecureProtocol.h
 *
 */

#ifndef PROCESSOR_INSECUREPROTOCOL_H_
#define PROCESSOR_INSECUREPROTOCOL_H_

#include <vector>
using namespace std;

#include "DummyProtocol.h"
#include "Protocols/MAC_Check.hpp"

class Player;
class DataPositions;
class ThreadQueues;

template<class T> class SubProcessor;

namespace GC
{
class NoShare;

template<class T> class ShareThread;
}

template<class T>
class InsecureMC : public MAC_Check_<T>
{
public:
    InsecureMC(const typename T::mac_key_type::Scalar& mac_key)
    : MAC_Check_<T>(mac_key)
    {
    }

    void prepare_open(const T& secret, int = -1)
    {
        this->values.push_back(secret.get_share());
        this->macs.push_back(secret.get_mac());
    }

    /**
     * Insecure. Just open the values.
    */
    void exchange(const Player& P)
    {
        vector<octetStream> oss;
        oss.resize(P.num_players());
        oss[P.my_num()].reset_write_head();
        oss[P.my_num()].reserve(this->values.size() * T::open_type::size());

        for (auto& x : this->values)
            x.pack(oss[P.my_num()]);
        oss[P.my_num()].append(0);

        P.unchecked_broadcast(oss);

        direct_add_openings<typename T::open_type>(this->values, P, oss);

        this->popen_cnt += this->values.size();
    }
    void CheckFor(const typename T::open_type&, const vector<T>&, const Player&) {
    }

    void Check (const Player& P) {

    }

    DummyMC<typename T::part_type>& get_part_MC()
    {
        return *new DummyMC<typename T::part_type>;
    }

    int number()
    {
        return 0;
    }
};

// template<class T>
// class DummyProtocol : public ProtocolBase<T>
// {
// public:
//     Player& P;
//     int counter;

//     static int get_n_relevant_players()
//     {
//         throw not_implemented();
//     }

//     static void multiply(vector<T>, vector<pair<T, T>>, int, int, SubProcessor<T>)
//     {
//     }

//     DummyProtocol(Player& P) :
//             P(P)
//     {
//     }

//     void init_mul()
//     {
//     }
//     void prepare_mul(const T&, const T&, int = 0)
//     {
//         throw not_implemented();
//     }
//     void exchange()
//     {
//         throw not_implemented();
//     }
//     T finalize_mul(int = 0)
//     {
//         throw not_implemented();
//         return {};
//     }
//     void check()
//     {
//     }
// };

template<class T>
class InsecureLivePrep2PC : public DummyLivePrep<T>
{
public:

    typename T::MAC_Check* MC;
    PRNG prng;
    Player* P;
    typename T::mac_key_type::Scalar revealed_key;

    // DummyLivePrep(DataPositions& usage, GC::ShareThread<T>&) :
    //         Preprocessing<T>(usage)
    // {
    // }
    InsecureLivePrep2PC(DataPositions& usage, bool = true) :
            DummyLivePrep<T>(usage)
    {
        prng.SetSeed((const unsigned char*) "insecure");
    }

    // DummyLivePrep(SubProcessor<T>*, DataPositions& usage) :
    //         Preprocessing<T>(usage)
    // {
    // }

    void set_protocol(typename T::Protocol& prot)
    {
        P = prot.get_player();
        std::cout << "Init player number: " << P->my_num() << std::endl;
        MC = prot.get_mc();
        set_mc(MC);
    }

    void set_mc(typename T::MAC_Check* MC) {
        octetStream os;
        const auto& mac_key = MC->get_alphai();
        mac_key.pack(os);
        P->exchange_relative(-1, os);
        typename T::mac_key_type::Scalar other_mac_key;
        other_mac_key.unpack(os);

        revealed_key = mac_key + other_mac_key;
        std::cout << "Revealed mac key: " << revealed_key << std::endl;
    }

    void get_three_no_count(Dtype dtype, T& a, T& b, T& c) {
        typedef typename T::open_type U;
        typedef typename T::mac_type W;
        array<array<U, 3>, 2> plain_triples;
        plain_triples[0][0].randomize(prng);
        plain_triples[0][1].randomize(prng);
        plain_triples[0][2].randomize(prng);
        plain_triples[1][0].randomize(prng);
        plain_triples[1][1].randomize(prng);
        plain_triples[1][2] = 
            U(plain_triples[0][0] + plain_triples[1][0]) * 
            U(plain_triples[0][1] + plain_triples[1][1]) - 
            plain_triples[0][2];
        
        array<array<W, 3>, 2> mac_triples;
        mac_triples[0][0].randomize(prng);
        mac_triples[0][1].randomize(prng);
        mac_triples[0][2].randomize(prng);
        mac_triples[1][0] = U(plain_triples[0][0] + plain_triples[1][0]) * revealed_key - mac_triples[0][0];
        mac_triples[1][1] = U(plain_triples[0][1] + plain_triples[1][1]) * revealed_key - mac_triples[0][1];
        mac_triples[1][2] = U(plain_triples[0][2] + plain_triples[1][2]) * revealed_key - mac_triples[0][2];

        int i = P->my_num();
        a = {plain_triples[i][0], mac_triples[i][0]};
        b = {plain_triples[i][1], mac_triples[i][1]};
        c = {plain_triples[i][2], mac_triples[i][2]};
    }

    void get_input_no_count(T& r_share, typename T::open_type& r , int player) {
        typedef typename T::open_type U;
        typedef typename T::mac_type W;

        r.randomize(prng);
        
        array<U, 2> value_shares;
        array<W, 2> mac_shares;
        value_shares[0].randomize(prng);
        value_shares[1] = r - value_shares[0];
        mac_shares[0].randomize(prng);
        mac_shares[1] = r * revealed_key - mac_shares[0];

        // `player` denotes the input's owner, but here we should use the share's owner
        r_share = {value_shares[P->my_num()], mac_shares[P->my_num()]};
    }
};

// template<class V>
// class NotImplementedInput
// {
// public:
//     template<class T, class U>
//     NotImplementedInput(const T& proc, const U& MC)
//     {
//         (void) proc, (void) MC;
//     }
//     template<class T, class U, class W>
//     NotImplementedInput(const T&, const U&, const W&)
//     {
//     }
//     template<class T>
//     NotImplementedInput(const T&)
//     {
//     }
//     void start(int n, vector<int> regs)
//     {
//         (void) n, (void) regs;
//         throw not_implemented();
//     }
//     void stop(int n, vector<int> regs)
//     {
//         (void) n, (void) regs;
//         throw not_implemented();
//     }
//     void start(int n, int m)
//     {
//         (void) n, (void) m;
//         throw not_implemented();
//     }
//     void stop(int n, int m)
//     {
//         (void) n, (void) m;
//         throw not_implemented();
//     }
//     template<class T>
//     static void input(SubProcessor<V>& proc, vector<int> regs, int)
//     {
//         (void) proc, (void) regs;
//         throw not_implemented();
//     }
//     static void input_mixed(SubProcessor<V>, vector<int>, int, int)
//     {
//     }
//     void reset_all(Player& P)
//     {
//         (void) P;
//         throw not_implemented();
//     }
//     template<class U>
//     void add_mine(U a, int b = 0)
//     {
//         (void) a, (void) b;
//         throw not_implemented();
//     }
//     void add_other(int)
//     {
//         throw not_implemented();
//     }
//     template<class U>
//     void add_from_all(U)
//     {
//         throw not_implemented();
//     }
//     void exchange()
//     {
//         throw not_implemented();
//     }
//     V finalize(int a, int b = 0)
//     {
//         (void) a, (void) b;
//         throw not_implemented();
//     }
//     static void raw_input(SubProcessor<V>&, vector<int>, int)
//     {
//         throw not_implemented();
//     }
//     static void input_mixed(SubProcessor<V>&, vector<int>, int, bool)
//     {
//         throw not_implemented();
//     }
// };

// class NotImplementedOutput
// {
// public:
//     template<class T>
//     NotImplementedOutput(SubProcessor<T>& proc)
//     {
//         (void) proc;
//     }

//     void start(int player, int target, int source)
//     {
//         (void) player, (void) target, (void) source;
//         throw not_implemented();
//     }
//     void stop(int player, int source, int)
//     {
//         (void) player, (void) source;
//     }
// };

#endif /* PROCESSOR_INSECUREPROTOCOL_H_ */
