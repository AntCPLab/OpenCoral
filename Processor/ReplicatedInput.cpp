/*
 * ReplicatedInput.cpp
 *
 */

#include "ReplicatedInput.h"
#include "Processor.h"

template<class T>
void ReplicatedInput<T>::start(int player, int n_inputs)
{
    assert(T::length == 2);

    shares.resize(n_inputs);

    if (player == proc.P.my_num())
    {
        vector<octetStream> os(2);

        for (int i = 0; i < n_inputs; i++)
        {
            typename T::value_type t;
            this->buffer.input(t);
            T& my_share = shares[i];
            my_share[0].randomize(proc.Proc.secure_prng);
            my_share[1] = t - my_share[0];
            for (int j = 0; j < 2; j++)
            {
                my_share[j].pack(os[j]);
            }
        }

        proc.P.send_relative(os);
        this->values_input += n_inputs;
    }
}

template<class T>
void ReplicatedInput<T>::stop(int player, vector<int> targets)
{
    if (proc.P.my_num() == player)
    {
        for (unsigned int i = 0; i < targets.size(); i++)
            proc.get_S_ref(targets[i]) = shares[i];
    }
    else
    {
        octetStream o;
        this->timer.start();
        proc.P.receive_player(player, o, true);
        this->timer.stop();
        for (unsigned int i = 0; i < targets.size(); i++)
        {
            typename T::value_type t;
            t.unpack(o);
            int j = proc.P.get_offset(player) == 2;
            T share;
            share[j] = t;
            share[1 - j] = 0;
            this->proc.get_S_ref(targets[i]) = share;
        }
    }
}

template class ReplicatedInput<Rep3Share>;
