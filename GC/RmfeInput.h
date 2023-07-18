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
    RmfeInput(MAC_Check& MC, Preprocessing<T>& prep, Player& P);

    RmfeInput(SubProcessor<T>& proc, MAC_Check&);

    void reset(int player);

    void add_mine(const open_type& input, int n_bits = -1);
    void add_mine_decoded(const BitVec& input, int n_bits = -1);
    void add_other(int player, int n_bits = -1);

    void send_mine();

    T finalize_mine();
    void finalize_other(int player, T& target, octetStream& o, int n_bits = -1);

    // void reset(int player)
    // {
    //     part_input.reset(player);
    // }

    // void add_mine(const typename T::clear& input, int n_bits)
    // {
    //     if (n_bits == -1)
    //         n_bits = T::default_length;

    //     (void) n_bits;
    //     int player = P.my_num();
    //     shares[player].push_back({});
    //     T& share = shares[player].back();
    //     prep.get_input(share, rr, player);
    //     t = input - rr;
    //     t.pack(this->os[player]);
    //     share += T::constant(t, player, MC.get_alphai());
    //     this->values_input++;
    // }

    // void add_other(int player, int n_bits)
    // {
    //     if (n_bits == -1)
    //         n_bits = T::default_length;
    //     for (int i = 0; i < n_bits; i++)
    //         part_input.add_other(player);
    // }

    // void send_mine()
    // {
    //     part_input.send_mine();
    // }

    // void exchange()
    // {
    //     part_input.exchange();
    // }

    // T finalize_mine()
    // {
    //     T res;
    //     res.resize_regs(input_lengths.front());
    //     for (int i = 0; i < input_lengths.front(); i++)
    //         res.get_reg(i) = part_input.finalize_mine();
    //     input_lengths.pop_front();
    //     return res;
    // }

    // void finalize_other(int player, T& target, octetStream&, int n_bits)
    // {
    //     if (n_bits == -1)
    //         n_bits = T::default_length;
    //     target.resize_regs(n_bits);
    //     for (int i = 0; i < n_bits; i++)
    //         part_input.finalize_other(player, target.get_reg(i),
    //                 part_input.InputBase<typename T::part_type>::os[player]);
    // }
};

} /* namespace GC */

#endif /* GC_RMFEINPUT_H_ */
