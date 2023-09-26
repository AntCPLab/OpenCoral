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
#include "Protocols/ReplicatedPrep.hpp"

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

    P = protocol.get_player();

    int tinyot_batch_size = triple_generator->nTriplesPerLoop * T::default_length;
    tinyot2rmfe = new RmfeShareConverter<TinyOTShare>(*P);
    tinyot2rmfe->get_src_prep()->set_batch_size(tinyot_batch_size);
}

// template<class T>
// void RmfeSharePrep<T>::set_mc(typename T::MAC_Check* MC) {
//     revealed_key = reveal(P, MC->get_alphai());
// }

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
    const int s = 40;
    auto n = triple_generator->nTriplesPerLoop + s;
    auto tinyot_prep = tinyot2rmfe->get_src_prep();

    int l = T::default_length;
    // triple storage arranged as: n x 3 x l
    vector<TinyOTShare> tinyot_shares(n * 3 * l);
    // triple storage arranged as: n x 3
    vector<T> rmfe_shares;

    // Generate tinyot triples
    for(int i = 0; i < n; i++) {
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
    assert((int)rmfe_shares.size() == n * 3);

    // Construct random encoded inputs that decode to the same raw input
    auto& MC = ShareThread<typename T::whole_type>::s().MC->get_part_MC();
    typename T::Input input(MC, *this, *P);
    input.reset_all(*P);
    for(int i = 0; i < n; i++) {
        long raw_a = 0, raw_b = 0;
        for(int j = 0; j < l; j++) {
            raw_a ^= ((long) typename T::clear(tinyot_shares[i* 3 * l + j].get_bit(0).get_share()).get_bit(0)) << j;
            raw_b ^= ((long) typename T::clear(tinyot_shares[i* 3 * l + l + j].get_bit(0).get_share()).get_bit(0)) << j;
        }
        input.add_from_all_encoded(T::open_type::random_preimage(typename T::clear(raw_a)));
        input.add_from_all_encoded(T::open_type::random_preimage(typename T::clear(raw_b)));
    }
    input.exchange();
    vector<T> random_a(n), random_b(n);
    for(int i = 0; i < n; i++) {
        random_a[i] = input.finalize(0) + input.finalize(1);
        random_b[i] = input.finalize(0) + input.finalize(1);
    }

    // Sacrifice
    GlobalPRNG prng(*P);
    vector<T> y(s), y_prime(s), z(s), z_prime(s);
    vector<typename T::open_type> y_open(s), y_prime_open(s), z_open(s), z_prime_open(s);
    for(int i = 0; i < s; i++) {
        y[i] = random_a[n - s + i];
        y_prime[i] = rmfe_shares[(n-s+i) * 3];
        z[i] = random_b[n - s + i];
        z_prime[i] = rmfe_shares[(n-s+i) * 3 + 1];
        for (int j = 0; j < n - s; j++) {
            if (prng.get_bit()) {
                y[i] += random_a[j];
                y_prime[i] += rmfe_shares[j*3];
                z[i] += random_b[j];
                z_prime[i] += rmfe_shares[j*3 + 1];
            }
        }
    }
    MC.POpen(y_open, y, *P);
    MC.POpen(y_prime_open, y_prime, *P);
    MC.POpen(z_open, z, *P);
    MC.POpen(z_prime_open, z_prime, *P);

    for (int i = 0; i < s; i++) {
        if (T::open_type::tau(y_open[i]) != y_prime_open[i])
            throw runtime_error("Inconsistency found for [a] share");
        if (T::open_type::tau(z_open[i]) != z_prime_open[i])
            throw runtime_error("Inconsistency found for [b] share");
    }

    for(int i = 0; i < n-s; i++) {
        this->quintuples.push_back({{random_a[i], random_b[i], rmfe_shares[i*3 + 2], rmfe_shares[i*3], rmfe_shares[i*3 + 1]}});
    }

    print_general("Generate RMFE quintuples", n-s);
}

template<class T>
void RmfeSharePrep<T>::buffer_personal_quintuples(size_t batch_size, ThreadQueues* queues)
{
    TripleShuffleSacrifice<T> sacri;
    // [zico] `sacri.C` here might be able to optimize, because the # of bit triples is actually sacri.C * default_length
    batch_size = max(batch_size, (size_t)sacri.minimum_n_outputs()) + sacri.C;
    vector<array<T, 5>> quintuples(batch_size);

    if (queues)
    {
        PersonalQuintupleJob job(&quintuples, input_player);
        int start = queues->distribute(job, batch_size);
        buffer_personal_quintuples(quintuples, start, batch_size);
        if (start)
            queues->wrap_up(job);
    }
    else
        buffer_personal_quintuples(quintuples, 0, batch_size);

    auto &party = ShareThread<typename T::whole_type>::s();
    assert(party.P != 0);
    assert(party.MC != 0);
    auto& MC = party.MC->get_part_MC();
    auto& P = *party.P;
    GlobalPRNG G(P);
    vector<T> shares;
    for (int i = 0; i < sacri.C; i++)
    {
        int challenge = G.get_uint(quintuples.size());
        for (auto& x : quintuples[challenge])
            shares.push_back(x);
        quintuples.erase(quintuples.begin() + challenge);
    }
    PointerVector<typename T::open_type> opened;
    MC.POpen(opened, shares, P);
    for (int i = 0; i < sacri.C; i++)
    {
        array<typename T::open_type, 5> quintuple({{opened.next(), opened.next(),
            opened.next(), opened.next(), opened.next()}});
        typename T::clear a(quintuple[0]), b(quintuple[1]), c(quintuple[2]);
        // Check triple EQ
        if (a * b != c)
        {
            cout << c << " != " << a * b << " = "
                    << a << " * " << b << endl;
            throw runtime_error("personal triple incorrect");
        }
        // Check normality
        if (typename T::open_type(a) != quintuple[3] || typename T::open_type(b) != quintuple[4])
        {
            throw runtime_error("personal quintuple normality incorrect");
        }
    }

    this->quintuples.insert(this->quintuples.end(), quintuples.begin(), quintuples.end());
}

template<class T>
void RmfeSharePrep<T>::buffer_personal_quintuples(vector<array<T, 5>>& quintuples, size_t begin, size_t end) {
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
        if (P.my_num() == input_player) {
            typename T::open_type x0 = G.get<typename T::open_type>(), x1 = G.get<typename T::open_type>();
            typename T::clear x0_clear = typename T::clear(x0), x1_clear = typename T::clear(x1);
            input.add_mine(x0, T::default_length);
            input.add_mine(x1, T::default_length);
            input.add_mine(x0_clear * x1_clear, T::default_length);
            input.add_mine(typename T::open_type(x0_clear), T::default_length);
            input.add_mine(typename T::open_type(x1_clear), T::default_length);
        }
        else {
            input.add_other(input_player);
            input.add_other(input_player);
            input.add_other(input_player);
            input.add_other(input_player);
            input.add_other(input_player);
        }
    }
    input.exchange();
    for (size_t i = begin; i < end; i++) {
        quintuples[i][0] = input.finalize(input_player, T::default_length);
        quintuples[i][1] = input.finalize(input_player, T::default_length);
        quintuples[i][2] = input.finalize(input_player, T::default_length);
        quintuples[i][3] = input.finalize(input_player, T::default_length);
        quintuples[i][4] = input.finalize(input_player, T::default_length);
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
    vector<T> randoms(n);

    SeededPRNG prng;
    for(int i = 0; i < n; i++) {
        // typename T::clear r = prng.get<typename T::clear>();
        typename T::open_type r = prng.get<typename T::open_type>();
        typename T::open_type normal = T::open_type::tau(r);
        input.add_from_all_encoded(r);
        input.add_from_all_encoded(normal);
    }
    input.exchange();

    for(int i = 0; i < n; i++) {
        randoms[i] = input.finalize(0) + input.finalize(1);
        normals[i] = input.finalize(0) + input.finalize(1);
    }

    // Step 2: Sacrifice
    vector<T> b(NORMAL_SACRIFICE);
    vector<T> c(NORMAL_SACRIFICE);
    GlobalPRNG G(P);
    for (int i = 0; i < NORMAL_SACRIFICE; i++) {
        b[i] = randoms[u + i];
        c[i] = normals[u + i];
        for (int j = 0; j < u; j++) {
            if (G.get_bit()) {
                b[i] += randoms[j];
                c[i] += normals[j];
            }
        }
    }
    vector<typename T::open_type> b_opened;
    vector<typename T::open_type> c_opened;
    MC->POpen(b_opened, b, P);
    MC->POpen(c_opened, c, P);
    for (int i = 0; i < NORMAL_SACRIFICE; i++) {
        if (!(T::open_type::tau(b_opened[i]) == c_opened[i])) {
            throw runtime_error("Fail checking normality of element");
        }
    }

    MC->Check(P);

    for (int i = 0; i < u; i++)
        this->normals.push_back({{randoms[i], normals[i]}});
    // this->normals.insert(this->normals.end(), normals.begin(), normals.begin() + u);
    delete MC;

    print_general("Generate random normal elements", u);
}

// template<class T>
// array<T, 5> RmfeSharePrep<T>::get_quintuple(int n_bits) {
//     // Treat quintuple as triple in counting
//     count(DATA_TRIPLE, n_bits);
//     waste(DATA_TRIPLE, T::default_length - n_bits);
//     n_bits = T::default_length;
//     return get_quintuple_no_count(n_bits);
// }

// template<class T>
// array<T, 5> RmfeSharePrep<T>::get_quintuple_no_count(int n_bits) {
//     assert(T::default_length == n_bits or not do_count);

//     if (quintuples.empty())
//     {
//         InScope in_scope(this->do_count, false, *this);
//         // [zico] We reuse the buffer_triples API, but actually it is buffering quintuples
//         buffer_triples();
//         assert(not quintuples.empty());
//     }

//     array<T, 5> res = quintuples.back();
//     quintuples.pop_back();
//     return res;
// }

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
