/*
 * RmfeBeaver.h
 *
 */

#ifndef PROTOCOLS_RMFE_BEAVER_H_
#define PROTOCOLS_RMFE_BEAVER_H_

#include <vector>
#include <array>

#include "Replicated.h"
#include "Processor/Data_Files.h"
#include "Math/mfe.h"
#include "Protocols/ProtocolGlobalInit.h"
#include "TinyOT/tinyotshare.h"

template<class T> class SubProcessor;
template<class T> class MAC_Check_Base;
class Player;

/**
 * RMFE Beaver multiplication
 */
template<class T>
class RmfeBeaver : public ProtocolBase<T>
{
protected:

    vector<T> shares;
    vector<typename T::open_type> opened;
    // vector<array<T, 3>> triples;
    vector<array<T, 5>> quintuples;
    vector<int> lengths;
    typename vector<typename T::open_type>::iterator it;
    // typename vector<array<T, 3>>::iterator triple;
    typename vector<array<T, 5>>::iterator quintuple;
    Preprocessing<T>* prep;
    typename T::MAC_Check* MC;

    vector<array<T, 2>> normals;
    vector<typename T::open_type> constants;
    typename vector<array<T, 2>>::iterator normal;
    typename vector<typename T::open_type>::iterator constant;


public:
    static const bool uses_triples = true;

    Player& P;

    static void setup(Player& P);
    static void teardown();
    static void setup_rmfe();
    static void teardown_rmfe();
    static void setup_mfe();
    static void teardown_mfe();

    RmfeBeaver(Player& P) : prep(0), MC(0), P(P) {}

    typename T::Protocol branch();

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC);

    void init_mul();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    T finalize_mul(int n = -1);

    void init_mul_constant();
    void prepare_mul_constant(const T& x, const typename T::clear& y, int n = -1);
    void exchange_mul_constant();
    T finalize_mul_constant(int n = -1);

    void check();

    void start_exchange();
    void stop_exchange();

    int get_n_relevant_players() { return 1 + T::threshold(P.num_players()); }

    // int get_buffer_size() { return triples.size(); }
    int get_buffer_size() { return quintuples.size(); }

    Player* get_player() {
        return &P;
    }

    typename T::MAC_Check* get_mc() {
        return MC;
    }

};

#endif /* PROTOCOLS_RMFE_BEAVER_H_ */
