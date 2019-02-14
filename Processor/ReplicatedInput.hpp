/*
 * ReplicatedInput.cpp
 *
 */

#include "ReplicatedInput.h"
#include "Processor.h"


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
inline void ReplicatedInput<T>::add_mine(const typename T::clear& input)
{
    auto& shares = this->shares;
    shares.push_back({});
    T& my_share = shares.back();
    my_share[0].randomize(secure_prng);
    my_share[1] = input - my_share[0];
    for (int j = 0; j < 2; j++)
    {
        my_share[j].pack(os[j]);
    }
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
void PrepLessInput<T>::start(int player, int n_inputs)
{
    assert(processor != 0);
    auto& proc = *processor;
    reset(player);

    if (player == proc.P.my_num())
    {
        for (int i = 0; i < n_inputs; i++)
        {
            typename T::value_type t;
            this->buffer.input(t);
            add_mine(t);
        }

        send_mine();
    }
}

template<class T>
void PrepLessInput<T>::stop(int player, vector<int> targets)
{
    assert(processor != 0);
    auto& proc = *processor;
    if (proc.P.my_num() == player)
    {
        for (unsigned int i = 0; i < targets.size(); i++)
            proc.get_S_ref(targets[i]) = finalize_mine();
    }
    else
    {
        octetStream o;
        this->timer.start();
        proc.P.receive_player(player, o, true);
        this->timer.stop();
        for (unsigned int i = 0; i < targets.size(); i++)
            finalize_other(player, proc.get_S_ref(targets[i]), o);
    }
}

template<class T>
inline void ReplicatedInput<T>::finalize_other(int player, T& target,
        octetStream& o)
{
    typename T::value_type t;
    t.unpack(o);
    int j = P.get_offset(player) == 2;
    T share;
    share[j] = t;
    share[1 - j] = 0;
    target = share;
}

template<class T>
T PrepLessInput<T>::finalize_mine()
{
    return this->shares[this->i_share++];
}
