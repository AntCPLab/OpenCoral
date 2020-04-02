/*
 * ReplicatedInput.cpp
 *
 */

#ifndef PROTOCOLS_REPLICATEDINPUT_HPP_
#define PROTOCOLS_REPLICATEDINPUT_HPP_

#include "ReplicatedInput.h"
#include "Processor/Processor.h"

#include "Processor/Input.hpp"

template<class T>
void ReplicatedInput<T>::reset(int player)
{
    if (player == P.my_num())
    {
        this->shares.clear();
        this->i_share = 0;
        os.resize(2);
        for (auto& o : os)
            o.reset_write_head();
    }
}

template<class T>
inline void ReplicatedInput<T>::add_mine(const typename T::open_type& input, int n_bits)
{
    auto& shares = this->shares;
    shares.push_back({});
    T& my_share = shares.back();
    my_share[0].randomize(protocol.shared_prngs[0], n_bits);
    my_share[1] = input - my_share[0];
    my_share[1].pack(os[1], n_bits);
    this->values_input++;
}

template<class T>
void ReplicatedInput<T>::add_other(int player)
{
    (void) player;
}

template<class T>
void ReplicatedInput<T>::send_mine()
{
    P.send_relative(os);
}

template<class T>
void ReplicatedInput<T>::exchange()
{
    for (int i = 1; i < P.num_players(); i++)
    {
        P.pass_around(os[i - 1], InputBase<T>::os[P.get_player(-i)], i);
    }
}

template<class T>
inline void ReplicatedInput<T>::finalize_other(int player, T& target,
        octetStream& o, int n_bits)
{
    if (P.get_offset(player) == 1)
    {
        typename T::value_type t;
        t.unpack(o, n_bits);
        target[0] = t;
        target[1] = 0;
    }
    else
    {
        target[0] = 0;
        target[1].randomize(protocol.shared_prngs[1], n_bits);
    }
}

template<class T>
T PrepLessInput<T>::finalize_mine()
{
    return this->shares[this->i_share++];
}

#endif
