/*
 * CoralPrep.cpp
 *
 */

#ifndef PROTOCOLS_CoralPREP_HPP_
#define PROTOCOLS_CoralPREP_HPP_

#include "CoralPrep.h"
#include "GC/BitAdder.h"

#include "DabitSacrifice.hpp"
#include "Tools/Exceptions.h"
#include "Tools/debug.h"
#ifdef DETAIL_BENCHMARK
#include "Tools/performance.h"
#endif

template<class T>
CoralPrep<T>::CoralPrep(SubProcessor<T>* proc, DataPositions& usage) :
    BufferPrep<T>(usage),
    BitPrep<T>(proc, usage), RingPrep<T>(proc, usage),
    MaliciousDabitOnlyPrep<T>(proc, usage),
    MaliciousRingPrep<T>(proc, usage),
    MascotTriplePrep<T>(proc, usage),
    RingOnlyPrep<T>(proc, usage),
    Spdz2kPrep<T>(proc, usage),
    spdz2k2rmfe(0)
{
}

template<class T>
CoralPrep<T>::~CoralPrep() {
    if (spdz2k2rmfe)
        delete spdz2k2rmfe;
}

template<class T>
void CoralPrep<T>::set_protocol(typename T::Protocol& protocol) {
    Spdz2kPrep<T>::set_protocol(protocol);

    spdz2k2rmfe = new RmfeShareConverter<GC::Spdz2kBShare<T::s>>(protocol.P);
}

template<class T>
void CoralPrep<T>::get_dabit(T& a, typename T::bit_type& b) {
    throw invalid_pack_usage();
}

#ifdef SPDZ2K_BIT
/**
 * This version uses Spdz2k SP's method to generate randbit, convert to B share, and then convert to RMFE share.
 * But because Spdz2k's Input for boolean shares is too costly, this method turns out to use more communication.
*/
template<class T>
void CoralPrep<T>::buffer_dabits(ThreadQueues* queues) {
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Coral buffer_dabits", this->protocol->P.total_comm().sent);
#endif

    int dl = T::bit_type::default_length;
    int buffer_size = OnlineOptions::singleton.batch_size;
    buffer_size = buffer_size / dl * dl;

    vector<T> a_bits(buffer_size);
    vector<GC::Spdz2kBShare<T::s>> spdz2k_b_bits(buffer_size);
    for (int i = 0; i < buffer_size; i++) {
        // Get random bit in the Z2k domain
        this->get_one(DATA_BIT, a_bits[i]);
        // Spdz2k's local A2B
        spdz2k_b_bits[i].resize_regs(1);
        spdz2k_b_bits[i].get_reg(0) = Spdz2kShare<1, T::s>(a_bits[i]);
    }
    
    // Convert to Rmfe style
    vector<RmfeShare> rmfe_shares;
    spdz2k2rmfe->convert(rmfe_shares, spdz2k_b_bits);

    assert(rmfe_shares.size() * dl == a_bits.size());

    for (size_t i = 0; i < rmfe_shares.size(); i++) {
        dabitpack<T> dv;
        dv.second = rmfe_shares[i];
        for (int j = 0; j < dl; j++)
            dv.first.push_back(a_bits[i * dl + j]);
        this->dabitpacks.push_back(dv);
    }

#ifdef DETAIL_BENCHMARK
    perf.stop(this->protocol->P.total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

#else
template<class T>
void CoralPrep<T>::buffer_dabits(ThreadQueues* queues)
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Coral buffer_dabits", this->protocol->P.total_comm().sent);
#endif

    assert(this->proc != 0);
    size_t prev_size = this->dabitpacks.size();

    /* Version 1: Same as in Spdz2kPrep.hpp. This is the most efficient one. */
    vector<dabitpack<T>> check_dabits;
    int buffer_size = BaseMachine::batch_size<T>(DATA_DABIT, this->buffer_size);
    this->buffer_dabits_from_bits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(buffer_size, T::bit_type::default_length), queues);
    dabit_sacrifice.sacrifice_without_bit_check(this->dabitpacks, check_dabits,
            *this->proc, queues);

    /* Version 2: Same as in MaliciousRingPrep.hpp */
    // vector<dabitpack<T>> check_dabits;
    // this->buffer_dabits_without_check(check_dabits,
    //     dabit_sacrifice.minimum_n_inputs(this->buffer_size, T::bit_type::default_length), queues);
    // dabit_sacrifice.sacrifice_and_check_bits(this->dabitpacks, check_dabits, *this->proc, queues);

    print_general("Generate dabitpack", this->dabitpacks.size() - prev_size);

#ifdef DETAIL_BENCHMARK
    perf.stop(this->protocol->P.total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}
#endif


/**
 * Reference: RingOnlyPrep.hpp
*/
template<class T>
void CoralPrep<T>::buffer_dabits_from_bits_without_check(
        vector<dabitpack<T> >& dabits, int buffer_size, ThreadQueues*)
{
    typedef typename T::bit_type::part_type BT;
    int dl = BT::default_length;
    assert(buffer_size % dl == 0);
    int buffer_pack_size = buffer_size / dl;

    vector<dabitpack<T>> new_dabits;
    assert(this->proc != 0);
    auto& party = GC::ShareThread<typename T::bit_type>::s();
    typedef typename T::bit_type::part_type BT;
    SubProcessor<BT> bit_proc(party.MC->get_part_MC(),
            this->proc->bit_prep, this->proc->P);
    typename T::bit_type::part_type::Input input(bit_proc);
    input.reset_all(this->proc->P);
    BufferScope<T> scope(*this, buffer_size);
    for (int i = 0; i < buffer_pack_size; i++)
    {
        new_dabits.push_back({});
        auto& dp = new_dabits.back();
        typename BT::clear tmp;
        for (int j = 0; j < dl; j++) {
            T bit;
            this->get_one(DATA_BIT, bit);
            dp.push_a(bit);
            
            tmp += typename BT::clear(bit.get_share().get_bit(0)) << j;
        }
        input.add_from_all(tmp);
    }
    input.exchange();
    for (size_t i = 0; i < new_dabits.size(); i++)
        for (int j = 0; j < this->proc->P.num_players(); j++)
            new_dabits[i].second += input.finalize(j);
    dabits.insert(dabits.end(), new_dabits.begin(), new_dabits.end());
}

#endif
