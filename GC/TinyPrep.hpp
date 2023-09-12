/*
 * TinyPrep.cpp
 *
 */

#ifndef GC_TINYPREP_HPP_
#define GC_TINYPREP_HPP_

#include "TinierSharePrep.h"

#include "Protocols/MascotPrep.hpp"
#include "Protocols/ShuffleSacrifice.hpp"
#include "Protocols/MalRepRingPrep.hpp"
#ifdef SPDZ2K_SP
#include "Protocols/GeneralShareConverter.hpp"
#endif

namespace GC
{

template<class T>
void TinierSharePrep<T>::init_real(Player& P)
{
    assert(real_triple_generator == 0);
    auto& thread = ShareThread<secret_type>::s();
    real_triple_generator = new typename T::whole_type::TripleGenerator(
            BaseMachine::fresh_ot_setup(P), P.N, -1,
            OnlineOptions::singleton.batch_size, 1, params,
            thread.MC->get_alphai(), &P);
    real_triple_generator->multi_threaded = false;
}

template<class T>
void TinierSharePrep<T>::buffer_secret_triples()
{
    auto& thread = ShareThread<secret_type>::s();
    auto& triple_generator = real_triple_generator;
    assert(triple_generator != 0);
    params.generateBits = false;
    vector<array<T, 3>> triples;
    TripleShuffleSacrifice<T> sacrifice;
    size_t required;
    required = sacrifice.minimum_n_inputs_with_combining(
            BaseMachine::batch_size<T>(DATA_TRIPLE));
    triple_generator->set_batch_size(DIV_CEIL(required, 64));
    while (triples.size() < required)
    {
        triple_generator->generatePlainTriples();
        triple_generator->unlock();
        assert(triple_generator->plainTriples.size() != 0);
        for (size_t i = 0; i < triple_generator->plainTriples.size(); i++)
            triple_generator->valueBits[2].set_portion(i,
                    triple_generator->plainTriples[i][2]);
        triple_generator->run_multipliers({});
        assert(triple_generator->plainTriples.size() != 0);
        for (size_t i = 0; i < triple_generator->plainTriples.size(); i++)
        {
            int dl = secret_type::default_length;
            for (int j = 0; j < dl; j++)
            {
                triples.push_back({});
                for (int k = 0; k < 3; k++)
                {
                    auto& share = triples.back()[k];
                    share.set_share(
                            triple_generator->plainTriples.at(i).at(k).get_bit(
                                    j));
                    typename T::mac_type mac;
                    mac = thread.MC->get_alphai() * share.get_share();
                    for (auto& multiplier : triple_generator->ot_multipliers)
                        mac += multiplier->macs.at(k).at(i * dl + j);
                    share.set_mac(mac);
                }
            }
        }
    }
    sacrifice.triple_sacrifice(triples, triples,
            *thread.P, thread.MC->get_part_MC());
    sacrifice.triple_combine(triples, triples, *thread.P,
            thread.MC->get_part_MC());
    for (auto& triple : triples)
        this->triples.push_back(triple);
    print_general("Generate TinierShare triples", triples.size());
}

#ifdef SPDZ2K_SP
template<class T>
void TinierSharePrep<T>::buffer_secret_triples_spdz2ksp() {
    auto& nTriplesPerLoop = triple_generator->nTriplesPerLoop;
    auto tinyot_prep = tinyot2spdz2k->get_src_prep();

    int l = T::default_length;
    // triple storage arranged as: n x 3 x l
    vector<TinyOTShare> tinyot_shares(nTriplesPerLoop * 3 * l);
    // triple storage arranged as: n x 3
    vector<T> spdz2k_shares;

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

    // Convert tinyot shares to spdz2k shares
    tinyot2spdz2k->convert(spdz2k_shares, tinyot_shares);
    assert((int)spdz2k_shares.size() == nTriplesPerLoop * 3);

    for(int i = 0; i < nTriplesPerLoop; i++) {
        this->triples.push_back({{spdz2k_shares[i*3], spdz2k_shares[i*3 + 1], spdz2k_shares[i*3 + 2]}});
    }

    print_general("Generate Spdz2k boolean triples", nTriplesPerLoop);
}
#endif

} /* namespace GC */

#endif 