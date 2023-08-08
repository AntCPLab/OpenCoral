
#ifndef TINYOT_TINYOTINPUT_H__
#define TINYOT_TINYOTINPUT_H__

#include "Processor/Input.h"
#include "tinyotshare.h"

/**
 * Just a dummy input class
*/
class TinyOTInput : public InputBase<TinyOTShare>
{
    typedef TinyOTShare T;
    typedef typename TinyOTShare::open_type open_type;
    typedef typename TinyOTShare::clear clear;
    typedef typename TinyOTShare::MAC_Check MAC_Check;

public:
    TinyOTInput(SubProcessor<T>& proc) {}
    TinyOTInput(SubProcessor<T>& proc, MAC_Check& mc) {}
    TinyOTInput(SubProcessor<T>* proc, Player& P) {}
    TinyOTInput(MAC_Check& MC, Preprocessing<T>& prep, Player& P) {}

    void reset(int player) {}

    void add_mine(const open_type& input, int n_bits = -1) {}
    void add_other(int player, int n_bits = -1) {}

    void send_mine() {}

    void finalize_other(int player, T& target, octetStream& o, int n_bits = -1) {}
};


#endif 