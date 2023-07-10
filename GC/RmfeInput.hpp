/*
 * RmfeInput.hpp
 *
 */

#ifndef PROCESSOR_RMFEINPUT_HPP_
#define PROCESSOR_RMFEINPUT_HPP_

#include "RmfeInput.h"
#include "Processor.h"

#include "IntInput.h"
#include "FixInput.h"
#include "FloatInput.h"

#include "IntInput.hpp"


// template<class T>
// Input<T>::Input(SubProcessor<T>& proc) :
//         Input(proc, proc.MC)
// {
// }

template<class T>
RmfeInput<T>::RmfeInput(SubProcessor<T>& proc, MAC_Check& mc) :
        InputBase<T>(proc.Proc), proc(&proc), MC(mc), prep(proc.DataF), P(proc.P),
        shares(proc.P.num_players())
{
}

// template<class T>
// RmfeInput<T>::RmfeInput(SubProcessor<T>* proc, Player& P) :
//         InputBase<T>(proc->Proc), proc(proc), MC(proc->MC), prep(proc->DataF), P(
//                 proc->P), shares(P.num_players())
// {
//     assert (proc != 0);
// }

template<class T>
RmfeInput<T>::RmfeInput(MAC_Check& MC, Preprocessing<T>& prep, Player& P) :
        proc(0), MC(MC), prep(prep), P(P), shares(P.num_players())
{
}

template<class T>
void RmfeInput<T>::reset(int player)
{
    InputBase<T>::reset(player);
    shares[player].clear();
}

template<class T>
void RmfeInput<T>::add_mine(const open_type& input, int n_bits)
{
    if (n_bits > T::default_length)
        throw runtime_error("Cannot handle bits more than rmfe packing size");
    if (n_bits == -1)
        n_bits = T::default_length;
    NTL::vec_GF2 ntl_input;
    open_type encoded_input;
    conv(ntl_input, input, n_bits);
    conv(encoded_input, Gf2RMFE::s().encode(ntl_tmp));

    int player = P.my_num();
    shares[player].push_back({});
    T& share = shares[player].back();
    prep.get_input(share, rr, player);
    t = encoded_input - rr;
    t.pack(this->os[player]);
    share += T::constant(t, player, MC.get_alphai());
    this->values_input++;
}

template<class T>
void RmfeInput<T>::add_other(int player, int)
{
    open_type t;
    shares.at(player).push_back({});
    prep.get_input(shares[player].back(), t, player);
}

template<class T>
void RmfeInput<T>::send_mine()
{
    this->os[P.my_num()].append(0);
    P.send_all(this->os[P.my_num()]);
}

template<class T>
T RmfeInput<T>::finalize_mine()
{
    return shares[P.my_num()].next();
}

template<class T>
void RmfeInput<T>::finalize_other(int player, T& target,
        octetStream& o, int n_bits)
{
    (void) n_bits;
    target = shares[player].next();
    t.unpack(o);
    target += T::constant(t, P.my_num(), MC.get_alphai());
}

#endif
