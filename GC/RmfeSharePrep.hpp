/*
 * RmfeSharePrep.cpp
 *
 */

#ifndef GC_RMFESHARE_PREP_HPP_
#define GC_RMFESHARE_PREP_HPP_

#include "RmfeSharePrep.h"

#include "Protocols/TinyOt2Rmfe.h"
#include "PersonalPrep.hpp"
#include "Tools/debug.h"
#include "TinyOT/tinyotprep.hpp"
#include "TinyOT/tinyotshare.hpp"

namespace GC
{

template<class T>
RmfeSharePrep<T>::RmfeSharePrep(DataPositions& usage, int input_player) :
        PersonalPrep<T>(usage, input_player),
        triple_generator(0),
        tinyot2rmfe(0)
{
    prng.SetSeed((const unsigned char*) "insecure");
}

template<class T>
RmfeSharePrep<T>::RmfeSharePrep(SubProcessor<T>*, DataPositions& usage) :
        RmfeSharePrep(usage)
{
}

template<class T>
RmfeSharePrep<T>::~RmfeSharePrep()
{
    if (triple_generator)
        delete triple_generator;
    if (tinyot2rmfe)
        delete tinyot2rmfe;
}

template<class T>
void RmfeSharePrep<T>::set_protocol(typename T::Protocol& protocol)
{
    if (triple_generator)
    {
        assert(&triple_generator->get_player() == &protocol.P);
        return;
    }

    params.generateMACs = true;
    params.amplify = false;
    params.check = false;
    auto& thread = ShareThread<typename T::whole_type>::s();
    triple_generator = new typename T::TripleGenerator(
            BaseMachine::fresh_ot_setup(protocol.P), protocol.P.N, -1,
            OnlineOptions::singleton.batch_size, 1,
            params, thread.MC->get_alphai(), &protocol.P);
    triple_generator->multi_threaded = false;
    this->inputs.resize(thread.P->num_players());
    // init_real(protocol.P);

    Player* player = protocol.get_player();

    int tinyot_batch_size = triple_generator->nTriplesPerLoop * T::default_length;
    tinyot2rmfe = new RmfeShareConverter<TinyOTShare>(*player);
    tinyot2rmfe->get_src_prep()->set_batch_size(tinyot_batch_size);

    MC = protocol.get_mc();
}

template<class T>
void RmfeSharePrep<T>::set_mc(typename T::MAC_Check* MC) {
    revealed_key = reveal(P, MC->get_alphai());
}

template<class T>
void RmfeSharePrep<T>::buffer_inputs(int player) {
    auto& inputs = this->inputs;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    for (auto& x : triple_generator->inputs)
        inputs.at(player).push_back(x);
}

template<class T>
void RmfeSharePrep<T>::buffer_triples() {

    auto& nTriplesPerLoop = triple_generator->nTriplesPerLoop;
    auto tinyot_prep = tinyot2rmfe->get_src_prep();

    int l = T::default_length;
    // triple storage arranged as: n x 3 x l
    vector<TinyOTShare> tinyot_shares(nTriplesPerLoop * 3 * l);
    // triple storage arranged as: n x 3
    vector<T> rmfe_shares;

    // Generate tinyot triples
    for(int i = 0; i < nTriplesPerLoop; i++) {
        for(int j = 0; j < l; j++) {
            tinyot_prep->get_tinyot_triple(
                tinyot_shares[i * 3 * l + j].MAC, 
                tinyot_shares[i * 3 * l + j].KEY,
                tinyot_shares[i * 3 * l + l + j].MAC, 
                tinyot_shares[i * 3 * l + l + j].KEY,
                tinyot_shares[i * 3 * l + 2 * l + j].MAC, 
                tinyot_shares[i * 3 * l + 2 * l + j].KEY);
        }
    }

    // Convert tinyot shares to rmfe shares
    tinyot2rmfe->convert(rmfe_shares, tinyot_shares);
    assert((int)rmfe_shares.size() == nTriplesPerLoop * 3);

    for(int i = 0; i < nTriplesPerLoop; i++) {
        this->triples.push_back({{rmfe_shares[i*3], rmfe_shares[i*3 + 1], rmfe_shares[i*3 + 2]}});
    }

    print_general("Generate RMFE triples", nTriplesPerLoop);
}

template<class T>
void RmfeSharePrep<T>::buffer_personal_triples(size_t batch_size, ThreadQueues* queues)
{
    TripleShuffleSacrifice<T> sacri;
    // [zico] `sacri.C` here might be able to optimize, because the # of bit triples is actually sacri.C * default_length
    batch_size = max(batch_size, (size_t)sacri.minimum_n_outputs()) + sacri.C;
    vector<array<T, 3>> triples(batch_size);

    if (queues)
    {
        PersonalTripleJob job(&triples, input_player);
        int start = queues->distribute(job, batch_size);
        buffer_personal_triples(triples, start, batch_size);
        if (start)
            queues->wrap_up(job);
    }
    else
        buffer_personal_triples(triples, 0, batch_size);

    auto &party = ShareThread<typename T::whole_type>::s();
    assert(party.P != 0);
    assert(party.MC != 0);
    auto& MC = party.MC->get_part_MC();
    auto& P = *party.P;
    GlobalPRNG G(P);
    vector<T> shares;
    for (int i = 0; i < sacri.C; i++)
    {
        int challenge = G.get_uint(triples.size());
        for (auto& x : triples[challenge])
            shares.push_back(x);
        triples.erase(triples.begin() + challenge);
    }
    PointerVector<typename T::open_type> opened;
    MC.POpen(opened, shares, P);
    for (int i = 0; i < sacri.C; i++)
    {
        array<typename T::clear, 3> triple({{opened.next(), opened.next(),
            opened.next()}});
        if (triple[0] * triple[1] != triple[2])
        {
            cout << triple[2] << " != " << triple[0] * triple[1] << " = "
                    << triple[0] << " * " << triple[1] << endl;
            throw runtime_error("personal triple incorrect");
        }
    }

    this->triples.insert(this->triples.end(), triples.begin(), triples.end());
}

template<class T>
void RmfeSharePrep<T>::buffer_personal_triples(vector<array<T, 3>>& triples, size_t begin, size_t end) {
#ifdef VERBOSE_EDA
    fprintf(stderr, "personal triples %zu to %zu\n", begin, end);
    RunningTimer timer;
#endif
    auto& party = ShareThread<typename T::whole_type>::s();
    auto& MC = party.MC->get_part_MC();
    auto& P = *party.P;
    assert(input_player < P.num_players());
    typename T::Input input(MC, *this, P);
    SeededPRNG G;
    input.reset_all(P);
    for (size_t i = begin; i < end; i++)
    {
        typename T::clear x0 = G.get<typename T::clear>(), x1 = G.get<typename T::clear>();
        if (P.my_num() == input_player) {
            input.add_mine(x0, T::default_length);
            input.add_mine(x1, T::default_length);
            input.add_mine(x0 * x1, T::default_length);
        }
        else {
            input.add_other(input_player);
            input.add_other(input_player);
            input.add_other(input_player);
        }
    }
    input.exchange();
    for (size_t i = begin; i < end; i++) {
        triples[i][0] = input.finalize(input_player, T::default_length);
        triples[i][1] = input.finalize(input_player, T::default_length);
        triples[i][2] = input.finalize(input_player, T::default_length);
    }
#ifdef VERBOSE_EDA
    fprintf(stderr, "personal triples took %f seconds\n", timer.elapsed());
#endif
}

template<class T>
void RmfeSharePrep<T>::buffer_normals() {
    auto& party = ShareThread<typename T::whole_type>::s();
    auto& P = *party.P;
    auto MC = T::new_mc(party.MC->get_alphai());
    typename T::Input input(*MC, *this, P);
    input.reset_all(P);

    // Step 1: Construct normal elements
    int u = triple_generator->nTriplesPerLoop;
    int n = u + NORMAL_SACRIFICE;
    vector<T> normals(n);

    SeededPRNG prng;
    for(int i = 0; i < n; i++) {
        typename T::clear r = prng.get<typename T::clear>();
        input.add_from_all(r);
    }
    input.exchange();

    for(int i = 0; i < n; i++) {
        normals[i] = input.finalize(0) + input.finalize(1);
    }

    // Step 2: Sacrifice
    vector<T> b(NORMAL_SACRIFICE);
    GlobalPRNG G(P);
    for (int i = 0; i < NORMAL_SACRIFICE; i++) {
        b[i] = normals[u + i];
        for (int j = 0; j < u; j++) {
            if (G.get_bit())
                b[i] += normals[j];
        }
    }
    vector<typename T::open_type> b_opened;
    MC->POpen(b_opened, b, P);
    for (int i = 0; i < NORMAL_SACRIFICE; i++) {
        if (!b_opened[i].is_normal()) {
            throw runtime_error("Fail checking normality of element");
        }
    }

    MC->Check(P);

    this->normals.insert(this->normals.end(), normals.begin(), normals.begin() + u);
    delete MC;

    print_general("Generate random normal elements", u);
}

#ifdef INSECURE_RMFE_PREP
template<class T>
void RmfeSharePrep<T>::get_input_no_count(T& r_share, typename T::open_type& r , int player) {
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

template<class T>
void RmfeSharePrep<T>::get_three_no_count(Dtype dtype, T& a, T& b, T& c) {
    typedef typename T::raw_type V;
    typedef typename T::open_type U;
    typedef typename T::mac_type W;
    array<array<V, 3>, 2> raw_triples;
    raw_triples[0][0].randomize(prng, T::default_length);
    raw_triples[0][1].randomize(prng, T::default_length);
    raw_triples[0][2].randomize(prng, T::default_length);
    raw_triples[1][0].randomize(prng, T::default_length);
    raw_triples[1][1].randomize(prng, T::default_length);
    raw_triples[1][2] = 
        V(raw_triples[0][0] + raw_triples[1][0]) * 
        V(raw_triples[0][1] + raw_triples[1][1]) - 
        raw_triples[0][2];

    array<array<U, 3>, 2> plain_triples;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            plain_triples[i][j] = U(raw_triples[i][j]);
    
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
#endif

}

#endif
