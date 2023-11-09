/*
 * BitAdder.cpp
 *
 */

#include "BitAdder.h"

#include <assert.h>

#ifdef RMFE_UNIT
#include "GC/RmfeSharePrep.hpp"

template<>
void BitAdder::add(vector<vector<GC::RmfeShare>>& res, const vector<vector<vector<GC::RmfeShare>>>& summands,
        SubProcessor<GC::RmfeShare>& proc, int length, ThreadQueues* queues,
        int player) {
    typedef GC::RmfeShare T;
    assert(not summands.empty());
    assert(not summands[0].empty());

    res.resize(summands[0][0].size());

    if (queues)
    {
        assert(length == T::default_length);
        int n_available = queues->find_available();
        int n_per_thread = queues->get_n_per_thread(res.size());
        vector<vector<array<T, 5>>> quintuples(n_available);
        vector<void*> supplies(n_available);
        for (int i = 0; i < n_available; i++)
        {
            if (T::expensive_triples)
            {
                supplies[i] = &quintuples[i];
                for (size_t j = 0; j < n_per_thread * summands.size(); j++)
                    quintuples[i].push_back(proc.DataF.get_quintuple(T::default_length));
#ifdef VERBOSE_EDA
                cerr << "supplied " << quintuples[i].size() << endl;
#endif
            }
        }

        ThreadJob job(&res, &summands, T::default_length, player);
        int start = queues->distribute_no_setup(job, res.size(), 0, 1,
                &supplies);
        BitAdder().add(res, summands, start,
                summands[0][0].size(), proc, T::default_length);
        if (start)
            queues->wrap_up(job);
    }
    else
        add(res, summands, 0, res.size(), proc, length);
}

template<>
void BitAdder::add(vector<vector<GC::RmfeShare>>& res, const vector<vector<vector<GC::RmfeShare>>>& summands,
        size_t begin, size_t end, SubProcessor<GC::RmfeShare>& proc, int length,
        int input_begin, const void* supply) {
    typedef GC::RmfeShare T;
#ifdef VERBOSE_EDA
    fprintf(stderr, "add bits %lu to %lu\n", begin, end);
#endif

    if (input_begin < 0)
        input_begin = begin;

    int n_bits = summands.size();
    for (size_t i = begin; i < end; i++)
        res.at(i).resize(n_bits + 1);

    size_t n_items = end - begin;

    if (supply)
    {
#ifdef VERBOSE_EDA
        fprintf(stderr, "got supply\n");
#endif
        auto& s = *(vector<array<T, 5>>*) supply;
        assert(s.size() == n_items * n_bits);
        proc.DataF.push_quintuples(s);
    }

    assert (summands[0].size() == 2);

    vector<T> carries(n_items);
    vector<T> a(n_items), b(n_items);
    auto& protocol = proc.protocol;
    for (int i = 0; i < n_bits; i++)
    {
        assert(summands[i].size() == 2);
        assert(summands[i][0].size() >= input_begin + n_items);
        assert(summands[i][1].size() >= input_begin + n_items);

        for (size_t j = 0; j < n_items; j++)
        {
            a[j] = summands[i][0][input_begin + j];
            b[j] = summands[i][1][input_begin + j];
        }

        protocol.init_mul();
        for (size_t j = 0; j < n_items; j++)
        {
            res[begin + j][i] = a[j] + b[j] + carries[j];
            // full adder using MUX
            protocol.prepare_mul(a[j] + b[j], carries[j] + a[j], length);
        }
        protocol.exchange();
        for (size_t j = 0; j < n_items; j++)
            carries[j] = a[j] + protocol.finalize_mul(length);
    }

    for (size_t j = 0; j < n_items; j++)
        res[begin + j][n_bits] = carries[j];
}
#endif

