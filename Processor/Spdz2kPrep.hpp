/*
 * Spdz2kPrep.cpp
 *
 */

#include "Processor/Spdz2kPrep.h"

template<class T>
Spdz2kPrep<T>::Spdz2kPrep(SubProcessor<T>* proc, DataPositions& usage) :
        MascotPrep<T>(proc, usage)
{
    this->params.amplify = false;
    bit_MC = 0;
    bit_DataF = 0;
    bit_proc = 0;
    bit_prep = 0;
    bit_protocol = 0;
}

template<class T>
Spdz2kPrep<T>::~Spdz2kPrep()
{
    if (bit_prep != 0)
    {
        delete bit_prep;
        delete bit_proc;
        delete bit_DataF;
        delete bit_MC;
        delete bit_protocol;
    }
}

template<class T>
void Spdz2kPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    MascotPrep<T>::set_protocol(protocol);
    assert(this->proc != 0);
    auto& proc = this->proc;
    bit_MC = new typename BitShare::MAC_Check(proc->MC.get_alphai());
    // just dummies
    bit_pos = DataPositions(proc->P.num_players());
    bit_DataF = new Sub_Data_Files<BitShare>(0, 0, "", bit_pos, 0);
    bit_proc = new SubProcessor<BitShare>(proc->Proc, *bit_MC, *bit_DataF, proc->P);
    bit_prep = new MascotPrep<BitShare>(bit_proc, bit_pos);
    bit_prep->params.amplify = false;
    bit_protocol = new typename BitShare::Protocol(proc->P);
    bit_prep->set_protocol(*bit_protocol);
    bit_MC->set_prep(*bit_prep);
    this->proc->MC.set_prep(*this);
}

template<class T>
void Spdz2kPrep<T>::buffer_bits()
{
    assert(this->proc != 0);

#ifdef SPDZ2K_SIMPLE_BITS
    RingPrep<T>::buffer_bits();
    assert(this->protocol != 0);
    auto& protocol = *this->protocol;
    protocol.init_mul(this->proc);
    T one = typename T::super(1, protocol.P.my_num(), this->proc->MC.get_alphai());
    for (auto& bit : this->bits)
        // one of the two is not a zero divisor, so if the product is zero, one of them is too
        protocol.prepare_mul(one - bit, bit);
    protocol.exchange();
    vector<T> checks(this->bits.size());
    for (size_t i = 0; i < this->bits.size(); i++)
        checks.push_back(protocol.finalize_mul());
    vector<typename T::open_type> opened;
    this->proc->MC.POpen(opened, checks, protocol.P);
    this->proc->MC.Check(protocol.P);
    for (auto& x : opened)
        if (typename T::clear(x) != 0)
            throw runtime_error("not a bit");
#else
    vector<BitShare> squares, random_shares;
    typename BitShare::super one(1, bit_proc->P.my_num(), bit_MC->get_alphai());
    auto buffer_size = this->buffer_size;
    for (int i = 0; i < buffer_size; i++)
    {
        BitShare a, a2;
        bit_prep->get_two(DATA_SQUARE, a, a2);
        squares.push_back((a2 + a) * 4 + one);
        random_shares.push_back(2 * a + one);
    }
    vector<typename BitShare::open_type> opened;
    bit_MC->POpen(opened, squares, bit_proc->P);
    for (int i = 0; i < buffer_size; i++)
    {
        Z2<BitShare::k> root = opened[i];
        if (root.get_bit(0) != 1)
            throw runtime_error("square not odd");
        BitShare tmp = random_shares[i] * root.sqrRoot().invert() + one;
        this->bits.push_back(typename BitShare::super(tmp.get_share() >> 1, tmp.get_mac() >> 1));
    }
    bit_MC->Check(bit_proc->P);
#endif
}

template<class T>
size_t Spdz2kPrep<T>::data_sent()
{
    size_t res = MascotPrep<T>::data_sent();
    if (bit_prep)
        res += bit_prep->data_sent();
    return res;
}
