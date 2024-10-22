/*
 * Spdz2kPrep.cpp
 *
 */

#ifndef PROTOCOLS_SPDZ2KPREP_HPP_
#define PROTOCOLS_SPDZ2KPREP_HPP_

#include "Spdz2kPrep.h"
#include "GC/BitAdder.h"

#include "DabitSacrifice.hpp"
#include "RingOnlyPrep.hpp"
#include "Tools/debug.h"
#include "Tools/performance.h"

template<class T>
Spdz2kPrep<T>::Spdz2kPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage),
        BitPrep<T>(proc, usage), RingPrep<T>(proc, usage),
        MaliciousDabitOnlyPrep<T>(proc, usage),
        MaliciousRingPrep<T>(proc, usage),
        MascotTriplePrep<T>(proc, usage),
        RingOnlyPrep<T>(proc, usage)
{
    this->params.amplify = false;
    bit_MC = 0;
    bit_proc = 0;
    bit_prep = 0;
}

template<class T>
Spdz2kPrep<T>::~Spdz2kPrep()
{
    if (bit_prep != 0)
    {
        delete bit_proc;
        delete bit_prep;
        delete bit_MC;
    }
}

template<class T>
void Spdz2kPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    OTPrep<T>::set_protocol(protocol);
    assert(this->proc != 0);
    auto& proc = this->proc;
    bit_MC = new typename BitShare::MAC_Check(proc->MC.get_alphai());
    // just dummies
    bit_pos = DataPositions(proc->P.num_players());
    bit_prep = new MascotTriplePrep<BitShare>(bit_proc, bit_pos);
    bit_prep->params.amplify = false;
    bit_proc = new SubProcessor<BitShare>(*bit_MC, *bit_prep, proc->P);
    bit_MC->set_prep(*bit_prep);
}

template<class T>
void MaliciousRingPrep<T>::buffer_bits()
{
    assert(this->proc != 0);
    RingPrep<T>::buffer_bits_without_check();
    assert(this->protocol != 0);
    auto& protocol = *this->protocol;
    protocol.init_dotprod();
    auto one = T::constant(1, protocol.P.my_num(), this->proc->MC.get_alphai());
    GlobalPRNG G(protocol.P);
    for (auto& bit : this->bits)
        // one of the two is not a zero divisor, so if the product is zero, one of them is too
        protocol.prepare_dotprod(one - bit, bit * G.get<typename T::open_type>());
    protocol.next_dotprod();
    protocol.exchange();
    this->proc->MC.CheckFor(0, {protocol.finalize_dotprod(this->bits.size())},
            protocol.P);
}

template<class T>
void Spdz2kPrep<T>::buffer_bits()
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Spdz2k randbit", this->protocol->P.total_comm().sent);
#endif

#ifdef SPDZ2K_SIMPLE_BITS
    MaliciousRingPrep<T>::buffer_bits();
#else
    bits_from_square_in_ring(this->bits, this->buffer_size, bit_prep);
#endif

#ifdef DETAIL_BENCHMARK
    perf.stop(this->protocol->P.total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

template<class T, class U>
void bits_from_square_in_ring(vector<T>& bits, int buffer_size, U* bit_prep)
{
    buffer_size = BaseMachine::batch_size<T>(DATA_BIT, buffer_size);
    typedef typename U::share_type BitShare;
    typedef typename BitShare::open_type open_type;
    assert(bit_prep != 0);
    auto bit_proc = bit_prep->get_proc();
    assert(bit_proc != 0);
    auto bit_MC = &bit_proc->MC;
    vector<BitShare> squares, random_shares;
    auto one = BitShare::constant(1, bit_proc->P.my_num(), bit_MC->get_alphai());
    bit_prep->buffer_size = buffer_size;
    for (int i = 0; i < buffer_size; i++)
    {
        BitShare a, a2;
        bit_prep->get_two(DATA_SQUARE, a, a2);
        squares.push_back((a2 + a) * 4 + one);
        random_shares.push_back(a * 2 + one);
    }
    vector<typename BitShare::open_type> opened;
    bit_MC->POpen(opened, squares, bit_proc->P);
    for (int i = 0; i < buffer_size; i++)
    {
        auto& root = opened[i];
        if (root.get_bit(0) != 1)
            throw runtime_error("square not odd");
        BitShare tmp = random_shares[i] * open_type(root.sqrRoot().invert()) + one;
        bits.push_back(tmp >> 1);
    }
    bit_MC->Check(bit_proc->P);
}

#ifdef SPDZ2K_BIT
template<class T>
void Spdz2kPrep<T>::get_dabit(T& a, GC::TinySecret<T::s>& b)
{
    this->get_one(DATA_BIT, a);
    b.resize_regs(1);
    b.get_reg(0) = Spdz2kShare<1, T::s>(a);
}
#endif

template<class T>
void Spdz2kPrep<T>::buffer_dabits(ThreadQueues* queues)
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Dabits", this->protocol->P.total_comm().sent);
#endif

    assert(this->proc != 0);
    size_t prev_size = this->dabits.size();
    vector<dabit<T>> check_dabits;
    int buffer_size = BaseMachine::batch_size<T>(DATA_DABIT, this->buffer_size);
    this->buffer_dabits_from_bits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(buffer_size), queues);
    dabit_sacrifice.sacrifice_without_bit_check(this->dabits, check_dabits,
            *this->proc, queues);

    print_general("Generate dabit", this->dabits.size() - prev_size);

#ifdef DETAIL_BENCHMARK
    perf.stop(this->protocol->P.total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

template<class T>
void MascotPrep<T>::buffer_edabits(bool strict, int n_bits,
        ThreadQueues* queues)
{
    this->buffer_edabits_from_personal(strict, n_bits, queues);
}

template<class T>
void MaliciousRingPrep<T>::buffer_edabits_from_personal(bool strict, int n_bits,
        ThreadQueues* queues)
{
    buffer_edabits_from_personal<0>(strict, n_bits, queues,
            T::clear::characteristic_two);
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_edabits_from_personal(bool, int,
        ThreadQueues*, true_type)
{
    throw runtime_error("only implemented for integer-like domains");
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_edabits_from_personal(bool strict, int n_bits,
        ThreadQueues* queues, false_type)
{
    assert(this->proc != 0);
    typedef typename T::bit_type::part_type bit_type;
    vector<vector<bit_type>> bits;
    vector<T> sums;
#ifdef VERBOSE_EDA
    cerr << "Generate edaBits of length " << n_bits << " to sacrifice" << endl;
    Timer timer;
    timer.start();
#endif
    auto &party = GC::ShareThread<typename T::bit_type>::s();
    SubProcessor<bit_type> bit_proc(party.MC->get_part_MC(),
            this->proc->bit_prep, this->proc->P);
    int n_relevant = this->proc->protocol.get_n_relevant_players();
    vector<vector<vector<bit_type>>> player_bits(n_bits);
    for (int i = 0; i < n_relevant; i++)
    {
        vector<T> tmp;
        vector<vector<bit_type>> tmp_bits;
        this->buffer_personal_edabits(n_bits, tmp, tmp_bits, bit_proc,
                i, strict, queues);
        assert(not tmp.empty());
        sums.resize(tmp.size());
        for (size_t j = 0; j < tmp.size(); j++)
            sums[j] += tmp[j];
        for (int j = 0; j < n_bits; j++)
            player_bits[j].push_back(tmp_bits[j]);
    }

    RunningTimer add_timer;
    BitAdder().add(bits, player_bits, bit_proc, bit_type::default_length,
            queues);
    player_bits.clear();
#ifdef VERBOSE_EDA
    cerr << "Adding edaBits took " << add_timer.elapsed() << " seconds" << endl;
    cerr << "Done with generating edaBits after " << timer.elapsed()
            << " seconds" << endl;
    RunningTimer finalize_timer;
#endif

    if (T::bit_type::tight_packed) {
        vector<edabitpack<T>> checked;
        int dl = bit_type::default_length;
        assert(sums.size() == bits.size() * dl);
        for (size_t i = 0; i < sums.size() / dl; i++)
        {
            edabitpack<T> eb;
            for (int j = 0; j < dl; j++)
                eb.push_a(sums[i*dl + j]);
            for (auto& x : bits[i])
                eb.push_b(x);
            checked.push_back(eb);
        }
        sums.clear();
        sums.shrink_to_fit();
        bits.clear();
        bits.shrink_to_fit();
        if (strict)
            this->template sanitize<0>(checked, n_bits, -1, queues);
        auto& output = this->edabitpacks[{strict, n_bits}];
        for (auto& x : checked)
        {
            output.push_back(x);
        }
        print_general("Generate edabitpack", checked.size());
    }
    else {
        vector<edabit<T>> checked;
        for (size_t i = 0; i < sums.size(); i++)
        {
            checked.push_back({sums[i], {}});
            int i1 = i / bit_type::default_length;
            int i2 = i % bit_type::default_length;
            checked.back().second.reserve(bits.at(i1).size());
            for (auto& x : bits.at(i1))
                checked.back().second.push_back(x.get_bit(i2));
        }
        sums.clear();
        sums.shrink_to_fit();
        bits.clear();
        bits.shrink_to_fit();
        if (strict)
            this->template sanitize<0>(checked, n_bits, -1, queues);
        auto& output = this->edabits[{strict, n_bits}];
        for (auto& x : checked)
        {
            if (output.empty() or output.back().full())
                output.push_back(x);
            else
                output.back().push_back(x);
        }
        print_general("Generate edabit", checked.size());
    }
#ifdef VERBOSE_EDA
    cerr << "Finalizing took " << finalize_timer.elapsed() << " seconds" << endl;
#endif
}

template<class T>
void MaliciousRingPrep<T>::buffer_edabits(bool strict, int n_bits,
        ThreadQueues* queues)
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Edabits", this->protocol->P.total_comm().sent);
#endif

    RunningTimer timer;
#ifndef NONPERSONAL_EDA
    this->buffer_edabits_from_personal(strict, n_bits, queues);
#else
    assert(this->proc != 0);
    ShuffleSacrifice<T> shuffle_sacrifice;
    typedef typename T::bit_type::part_type bit_type;
    vector<vector<bit_type>> bits;
    vector<T> sums;
    this->buffer_edabits_without_check(n_bits, sums, bits,
            shuffle_sacrifice.minimum_n_inputs(), queues);
    vector<edabit<T>>& checked = this->edabits[{strict, n_bits}];
    shuffle_sacrifice.edabit_sacrifice(checked, sums, bits,
            n_bits, *this->proc, strict, -1, queues);
    if (strict)
        this->sanitize(checked, n_bits, -1, queues);
#endif
#ifdef VERBOSE_EDA
    cerr << "Total edaBit generation took " << timer.elapsed() << " seconds" << endl;
#endif

#ifdef DETAIL_BENCHMARK
    perf.stop(this->protocol->P.total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

#endif
