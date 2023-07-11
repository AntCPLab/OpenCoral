/*
 * RmfeMultiInput.h
 *
 */

#ifndef GC_RMFEMULTIINPUT_H_
#define GC_RMFEMULTIINPUT_H_

#include "Protocols/ReplicatedInput.h"

namespace GC
{

template<class T>
class RmfeMultiInput : public InputBase<T>
{
    typename T::part_type::Input part_input;
    deque<int> input_lengths;

public:
    RmfeMultiInput(typename T::MAC_Check& MC, Preprocessing<T>& prep, Player& P) :
            part_input(MC.get_part_MC(), prep.get_part(), P)
    {
        part_input.reset_all(P);
    }

    RmfeMultiInput(SubProcessor<T>& proc, typename T::MAC_Check&) :
            part_input(proc.MC, proc.DataF, proc.P)
    {
    }

    void reset(int player)
    {
        part_input.reset(player);
    }

    void add_mine(const typename T::clear& input, int n_bits)
    {
        if (n_bits == -1)
            n_bits = T::default_length;
        for (int i = 0; i < n_bits; i = i + T::part_type::default_length) {
            int l = std::min(n_bits - i, T::part_type::default_length);
            
            part_input.add_mine(typename T::clear(input >> i).mask(l), l);
        }
        input_lengths.push_back(n_bits);
    }

    void add_other(int player, int n_bits)
    {
        if (n_bits == -1)
            n_bits = T::default_length;
        for (int i = 0; i < n_bits; i = i + T::part_type::default_length)
            part_input.add_other(player);
    }

    void send_mine()
    {
        part_input.send_mine();
    }

    void exchange()
    {
        part_input.exchange();
    }

    T finalize_mine()
    {
        T res;
        res.resize_regs(input_lengths.front());
        int n_parts = (input_lengths.front() + T::part_type::default_length - 1) / T::part_type::default_length;
        for (int i = 0; i < n_parts; i++)
            res.get_reg(i) = part_input.finalize_mine();
        input_lengths.pop_front();
        return res;
    }

    void finalize_other(int player, T& target, octetStream&, int n_bits)
    {
        if (n_bits == -1)
            n_bits = T::default_length;
        int n_parts = (n_bits + T::part_type::default_length - 1) / T::part_type::default_length;
        target.resize_regs(n_parts);
        for (int i = 0; i < n_parts; i++)
            part_input.finalize_other(player, target.get_reg(i),
                    part_input.InputBase<typename T::part_type>::os[player]);
    }
};

} /* namespace GC */

#endif /* GC_RMFEMULTIINPUT_H_ */
