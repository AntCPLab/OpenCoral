/*
 * RmfeInput.h
 *
 */

#ifndef GC_RMFEINPUT_H_
#define GC_RMFEINPUT_H_

#include "Processor/Input.h"

namespace GC
{

template<class T>
class RmfeInput : public InputBase<T>
{
    typedef typename T::open_type open_type;
    typedef typename T::clear clear;
    typedef typename T::MAC_Check MAC_Check;

    SubProcessor<T>* proc;
    MAC_Check& MC;
    Preprocessing<T>& prep;
    Player& P;
    vector< PointerVector<T> > shares;
    open_type rr, t, xi;

public:
    RmfeInput(SubProcessor<T>& proc);
    RmfeInput(MAC_Check& MC, Preprocessing<T>& prep, Player& P);

    RmfeInput(SubProcessor<T>& proc, MAC_Check&);

    void reset(int player);

    void add_mine(const open_type& input, int n_bits = -1);
    void add_mine(const BitVec& input, int n_bits = -1);

    void add_other(int player, int n_bits = -1);

    /* NOTE: This hides the `add_from_all` function of InputBase. */
    void add_from_all(const BitVec& input, int n_bits = -1)
    {
        for (int i = 0; i < this->P.num_players(); i++)
            if (i == this->P.my_num())
                add_mine(input, n_bits);
            else
                add_other(i, n_bits);
    }

    void add_from_all_encoded(const typename T::open_type& input, int n_bits = -1) {
        for (int i = 0; i < this->P.num_players(); i++)
            if (i == this->P.my_num())
                add_mine(input, n_bits);
            else
                add_other(i, n_bits);
    }

    void send_mine();

    T finalize_mine();
    void finalize_other(int player, T& target, octetStream& o, int n_bits = -1);
};

} /* namespace GC */

#endif /* GC_RMFEINPUT_H_ */
