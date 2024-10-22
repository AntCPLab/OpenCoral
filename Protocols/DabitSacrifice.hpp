/*
 * DabitSacrifice.cpp
 *
 */

#ifndef PROTOCOLS_DABITSACRIFICE_HPP_
#define PROTOCOLS_DABITSACRIFICE_HPP_

#include "DabitSacrifice.h"
#include "BufferScope.h"
#include "Tools/PointerVector.h"
#include "Tools/debug.h"

#include <math.h>

template<class T>
DabitSacrifice<T>::DabitSacrifice() :
        S(OnlineOptions::singleton.security_parameter),
        n_masks(0), n_produced()
{
}

template<class T>
dabit<T>& operator+=(dabit<T>& x, const dabit<T>& y)
{
    x.first += y.first;
    x.second ^= y.second;
    return x;
}

template<class T>
void DabitSacrifice<T>::sacrifice_without_bit_check(vector<dabit<T> >& dabits,
        vector<dabit<T> >& check_dabits, SubProcessor<T>& proc,
        ThreadQueues*)
{
#ifdef VERBOSE_DABIT
    cerr << "Sacrificing daBits" << endl;
    Timer timer;
    timer.start();
#endif
    int n = check_dabits.size() - S;
    n_masks += S;
    assert(n > 0);
    GlobalPRNG G(proc.P);
    typedef typename T::bit_type::part_type BT;
    vector<T> shares;
    vector<BT> bit_shares;
    if (T::clear::N_BITS <= 0)
        dynamic_cast<BufferPrep<T>&>(proc.DataF).buffer_extra(DATA_BIT,
                S * (ceil(log2(n)) + S));
    for (int i = 0; i < S; i++)
    {
        dabit<T> to_check;
        for (int j = 0; j < n; j++)
        {
            if (G.get_bit())
                to_check += check_dabits[j];
        }
        to_check += check_dabits[n + i];
        T masked = to_check.first;
        if (T::clear::N_BITS > 0)
            masked = masked << (T::clear::N_BITS - 1);
        else
            for (int j = 0; j < ceil(log2(n)) + S; j++)
            {
                T tmp;
                proc.DataF.get_one(DATA_BIT, tmp);
                masked += tmp << (1 + j);
                n_masks++;
            }
        shares.push_back(masked);
        bit_shares.push_back(to_check.second);
    }
    auto& MC = proc.MC;
    auto& MCBB = *BT::new_mc(
            GC::ShareThread<typename T::bit_type>::s().MC->get_alphai());
    vector<typename T::open_type> opened;
    vector<typename BT::open_type> bit_opened;
    MC.POpen(opened, shares, proc.P);
    MCBB.POpen(bit_opened, bit_shares, proc.P);
    for (int i = 0; i < S; i++)
    {
        auto a = typename T::clear(opened[i]);
        if (T::clear::N_BITS > 0)
            a >>= (T::clear::N_BITS - 1);
        else
            a &= 1;
        auto b = bit_opened[i];
        if (a != b.get())
        {
            cerr << a << " != " << b << endl;
            throw Offline_Check_Error("daBit sacrifice");
        }
    }
    dabits.insert(dabits.end(), check_dabits.begin(), check_dabits.begin() + n);
    n_produced += n;
    MCBB.Check(proc.P);
    delete &MCBB;
#ifdef VERBOSE_DABIT
    cerr << "Done sacrificing daBits after " << timer.elapsed() << " seconds"
            << endl;
#endif
}

template<class T>
DabitSacrifice<T>::~DabitSacrifice()
{
#ifdef DABIT_WASTAGE
    if (n_produced > 0)
    {
        cerr << "daBit wastage: " << float(n_masks) / n_produced << endl;
    }
#endif
}

template<class T>
void DabitSacrifice<T>::sacrifice_and_check_bits(vector<dabit<T> >& dabits,
        vector<dabit<T> >& check_dabits, SubProcessor<T>& proc,
        ThreadQueues* queues)
{
    vector<dabit<T>> to_check;
    sacrifice_without_bit_check(to_check, check_dabits, proc, queues);
    typename T::Protocol protocol(proc.P);
    vector<pair<T, T>> multiplicands;
    for (auto& x : to_check)
        multiplicands.push_back({x.first, x.first});
    PointerVector<T> products(multiplicands.size());
    if (queues)
    {
        ThreadJob job(&products, &multiplicands);
        int start = queues->distribute(job, multiplicands.size());
        protocol.multiply(products, multiplicands, start, multiplicands.size(), proc);
        if (start)
            queues->wrap_up(job);
    }
    else
    {
        BufferScope<T> scope(proc.DataF, multiplicands.size());
        protocol.multiply(products, multiplicands, 0, multiplicands.size(), proc);
    }
    vector<T> check_for_zero;
    for (auto& x : to_check)
        check_for_zero.push_back(x.first - products.next());
    proc.MC.CheckFor(0, check_for_zero, proc.P);
    dabits.insert(dabits.end(), to_check.begin(), to_check.end());
}


template<class T>
void DabitSacrifice<T>::sacrifice_without_bit_check(vector<dabitpack<T> >& dabits,
        vector<dabitpack<T> >& check_dabits, SubProcessor<T>& proc,
        ThreadQueues*)
{
#ifdef VERBOSE_DABIT
    cerr << "Sacrificing daBits" << endl;
    Timer timer;
    timer.start();
#endif
    int dl = T::bit_type::default_length;
    int sacrificed_pack_n = DIV_CEIL(S, dl);
    int adjusted_S = sacrificed_pack_n * dl;
    
    int n = check_dabits.size() - sacrificed_pack_n;
    n_masks += adjusted_S;
    assert(n > 0);
    GlobalPRNG G(proc.P);
    typedef typename T::bit_type BT;
    vector<T> shares;
    vector<BT> bit_shares;
    if (T::clear::N_BITS <= 0)
        dynamic_cast<BufferPrep<T>&>(proc.DataF).buffer_extra(DATA_BIT,
                S * (ceil(log2(n)) + S));
    for (int i = 0; i < sacrificed_pack_n; i++)
    {
        dabitpack<T> to_check = check_dabits[n+i];
        for (int j = 0; j < n; j++)
        {
            typename BT::clear b = G.get<typename BT::clear>();
            to_check.second += check_dabits[j].second * typename BT::open_type(b);
            for (int h = 0; h < dl; h++) {
                if (b.get_bit(h))
                    to_check.first[h] += check_dabits[j].first[h];
            }
        }
        auto& masked = to_check.first;
        if (T::clear::N_BITS > 0)
            masked = masked << (T::clear::N_BITS - 1);
        else {
            // [zico] This branch needs to be checked for T which is not mod 2^k. I haven't verified it.
            for (size_t k = 0; k < masked.size(); k++) {
                for (int j = 0; j < ceil(log2(n)) + S; j++)
                {
                    T tmp;
                    proc.DataF.get_one(DATA_BIT, tmp);
                    masked[k] += tmp << (1 + j);
                    n_masks++;
                }
            }
        }
        shares.insert(shares.end(), masked.begin(), masked.end());
        bit_shares.push_back(to_check.second);
    }
    auto& MC = proc.MC;
    auto& MCBB = *BT::new_mc(
            GC::ShareThread<typename T::bit_type>::s().MC->get_alphai());
    vector<typename T::open_type> opened;
    vector<typename BT::open_type> bit_opened;
    MC.POpen(opened, shares, proc.P);
    MCBB.POpen(bit_opened, bit_shares, proc.P);
    for (int i = 0; i < sacrificed_pack_n; i++)
    {
        auto b = typename BT::clear(bit_opened[i]);

        for (int j = 0; j < dl; j++) {
            auto a = typename T::clear(opened[i * dl + j]);
            if (T::clear::N_BITS > 0)
                a >>= (T::clear::N_BITS - 1);
            else
                a &= 1;
            
            if (a != b.get_bit(j))
            {
                cerr << a << " != " << b.get_bit(j) << endl;
                throw Offline_Check_Error("daBit sacrifice");
            }
        }
    }
    dabits.insert(dabits.end(), check_dabits.begin(), check_dabits.begin() + n);
    n_produced += n;
    MCBB.Check(proc.P);
    delete &MCBB;
#ifdef VERBOSE_DABIT
    cerr << "Done sacrificing daBits after " << timer.elapsed() << " seconds"
            << endl;
#endif
}

template<class T>
void DabitSacrifice<T>::sacrifice_and_check_bits(vector<dabitpack<T> >& dabits,
        vector<dabitpack<T> >& check_dabits, SubProcessor<T>& proc,
        ThreadQueues* queues)
{
    vector<dabitpack<T>> to_check;
    sacrifice_without_bit_check(to_check, check_dabits, proc, queues);
    typename T::Protocol protocol(proc.P);
    vector<pair<T, T>> multiplicands;
    for (auto& x : to_check)
        for (auto& b : x.first)
            multiplicands.push_back({b, b});
    PointerVector<T> products(multiplicands.size());
    if (queues)
    {
        ThreadJob job(&products, &multiplicands);
        int start = queues->distribute(job, multiplicands.size());
        protocol.multiply(products, multiplicands, start, multiplicands.size(), proc);
        if (start)
            queues->wrap_up(job);
    }
    else
    {
        BufferScope<T> scope(proc.DataF, multiplicands.size());
        protocol.multiply(products, multiplicands, 0, multiplicands.size(), proc);
    }
    vector<T> check_for_zero;
    for (auto& x : to_check)
        for (auto& b : x.first) 
            check_for_zero.push_back(b - products.next());
    proc.MC.CheckFor(0, check_for_zero, proc.P);
    dabits.insert(dabits.end(), to_check.begin(), to_check.end());
}


#endif
