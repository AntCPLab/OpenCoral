/*
 * TinyPrep.cpp
 *
 */

#include "TinyPrep.h"

namespace GC
{

template<class T>
TinyPrep<T>::TinyPrep(DataPositions& usage, Thread<T>& thread) :
        BufferPrep<T>(usage), thread(thread), triple_generator(0),
        input_generator(0)
{
}

template<class T>
TinyPrep<T>::~TinyPrep()
{
    if (triple_generator)
        delete triple_generator;
    if (input_generator)
        delete input_generator;
}

template<class T>
void TinyPrep<T>::set_protocol(Beaver<T>& protocol)
{
    (void) protocol;
    params.generateMACs = true;
    params.amplify = false;
    params.check = false;
    triple_generator = new typename T::TripleGenerator(
            thread.processor.machine.ot_setups.at(thread.thread_num).get_fresh(),
            thread.master.N, thread.thread_num,
            thread.master.opts.batch_size,
            1, params, thread.MC->get_alphai(), thread.P);
    triple_generator->multi_threaded = false;
    input_generator = new typename T::part_type::TripleGenerator(
            thread.processor.machine.ot_setups.at(thread.thread_num).get_fresh(),
            thread.master.N, thread.thread_num,
            thread.master.opts.batch_size,
            1, params, thread.MC->get_alphai(), thread.P);
    input_generator->multi_threaded = false;
    thread.MC->get_part_MC().set_prep(*this);
}

template<class T>
void TinyPrep<T>::buffer_triples()
{
    auto& triple_generator = this->triple_generator;
    params.generateBits = false;
    vector<array<typename T::part_type::super, 3>> triples;
    ShuffleSacrifice<typename T::part_type::super> sacrifice;
    while (int(triples.size()) < sacrifice.minimum_n_inputs())
    {
        triple_generator->generatePlainTriples();
        triple_generator->unlock();
        assert(triple_generator->plainTriples.size() != 0);
        for (size_t i = 0; i < triple_generator->plainTriples.size(); i++)
            triple_generator->valueBits[2].set_portion(i,
                    triple_generator->plainTriples[i][2]);
        triple_generator->run_multipliers({});
        for (size_t i = 0; i < triple_generator->plainTriples.size(); i++)
        {
            for (int j = 0; j < T::default_length; j++)
            {
                triples.push_back({});
                for (int k = 0; k < 3; k++)
                {
                    auto& share = triples.back()[k];
                    share.set_share(
                            triple_generator->plainTriples.at(i).at(k).get_bit(
                                    j));
                    typename T::part_type::mac_type mac;
                    mac = thread.MC->get_alphai() * share.get_share();
                    for (auto& multiplier : triple_generator->ot_multipliers)
                        mac += multiplier->macs.at(k).at(i * T::default_length + j);
                    share.set_mac(mac);
                }
            }
        }
    }
    sacrifice.triple_sacrifice(triples, triples,
            *thread.P, thread.MC->get_part_MC());
    for (size_t i = 0; i < triples.size() / T::default_length; i++)
    {
        this->triples.push_back({});
        auto& triple = this->triples.back();
        for (auto& x : triple)
            x.resize_regs(T::default_length);
        for (int j = 0; j < T::default_length; j++)
        {
            auto& source_triple = triples[j + i * T::default_length];
            for (int k = 0; k < 3; k++)
                triple[k].get_reg(j) = source_triple[k];
        }
    }
}

template<class T>
void TinyPrep<T>::buffer_bits()
{
    auto tmp = BufferPrep<T>::get_random_from_inputs(thread.P->num_players());
    for (auto& bit : tmp.get_regs())
    {
        this->bits.push_back({});
        this->bits.back().resize_regs(1);
        this->bits.back().get_reg(0) = bit;
    }
}

template<class T>
void TinyPrep<T>::buffer_inputs(int player)
{
    auto& inputs = this->inputs;
    inputs.resize(thread.P->num_players());
    assert(this->input_generator);
    this->input_generator->generateInputs(player);
    for (size_t i = 0; i < this->input_generator->inputs.size() / T::default_length; i++)
    {
        inputs[player].push_back({});
        inputs[player].back().share.resize_regs(T::default_length);
        for (int j = 0; j < T::default_length; j++)
        {
            auto& source_input = this->input_generator->inputs[j
                    + i * T::default_length];
            inputs[player].back().share.get_reg(j) = res_type(source_input.share);
            inputs[player].back().value ^= typename T::open_type(
                    source_input.value.get_bit(0)) << j;
        }
    }
}

template<class T>
typename T::part_type::super GC::TinyPrep<T>::get_random()
{
    T tmp;
    this->get_one(DATA_BIT, tmp);
    return tmp.get_reg(0);
}

template<class T>
array<T, 3> TinyPrep<T>::get_triple(int n_bits)
{
    assert(n_bits > 0);
    while ((unsigned)n_bits > triple_buffer.size())
    {
        array<T, 3> tmp;
        this->get(DATA_TRIPLE, tmp.data());
        for (size_t i = 0; i < tmp[0].get_regs().size(); i++)
        {
            triple_buffer.push_back(
            { {tmp[0].get_reg(i), tmp[1].get_reg(i), tmp[2].get_reg(i)} });
        }
    }

    array<T, 3> res;
    for (int j = 0; j < 3; j++)
        res[j].resize_regs(n_bits);

    for (int i = 0; i < n_bits; i++)
    {
        for (int j = 0; j < 3; j++)
            res[j].get_reg(i) = triple_buffer.back()[j];
        triple_buffer.pop_back();
    }

    return res;
}

} /* namespace GC */
