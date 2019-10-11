/*
 * Spdz2kPrep.cpp
 *
 */

#include "Spdz2kPrep.h"

template<class T>
Spdz2kPrep<T>::Spdz2kPrep(SubProcessor<T>* proc, DataPositions& usage) :
        RingPrep<T>(proc, usage), MascotPrep<T>(proc, usage)
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
void MaliciousRingPrep<T>::buffer_bits()
{
    assert(this->proc != 0);
    RingPrep<T>::buffer_bits_without_check();
    assert(this->protocol != 0);
    auto& protocol = *this->protocol;
    protocol.init_mul(this->proc);
    T one(1, protocol.P.my_num(), this->proc->MC.get_alphai());
    for (auto& bit : this->bits)
        // one of the two is not a zero divisor, so if the product is zero, one of them is too
        protocol.prepare_mul(one - bit, bit);
    protocol.exchange();
    vector<T> checks(this->bits.size());
    for (size_t i = 0; i < this->bits.size(); i++)
        checks.push_back(protocol.finalize_mul());
    this->proc->MC.CheckFor(0, checks, protocol.P);
}

template<class T>
void Spdz2kPrep<T>::buffer_bits()
{
#ifdef SPDZ2K_SIMPLE_BITS
    MaliciousRingPrep<T>::buffer_bits();
#else
    bits_from_square_in_ring(this->bits, this->buffer_size, bit_prep);
#endif
}

template<class T, class U>
void bits_from_square_in_ring(vector<T>& bits, int buffer_size, U* bit_prep)
{
    typedef typename U::share_type BitShare;
    typedef typename BitShare::open_type open_type;
    assert(bit_prep != 0);
    auto bit_proc = bit_prep->get_proc();
    assert(bit_proc != 0);
    auto bit_MC = &bit_proc->MC;
    vector<BitShare> squares, random_shares;
    BitShare one(1, bit_proc->P.my_num(), bit_MC->get_alphai());
    for (int i = 0; i < buffer_size; i++)
    {
        BitShare a, a2;
        bit_prep->get_two(DATA_SQUARE, a, a2);
        squares.push_back((a2 + a) * 4 + one);
        random_shares.push_back(a * 2 + one);
    }
    vector<typename BitShare::open_type> opened;
    bit_MC->POpen(opened, squares, bit_proc->P);
    for (int i = 0; i < buffer_size; i++)
    {
        auto& root = opened[i];
        if (root.get_bit(0) != 1)
            throw runtime_error("square not odd");
        BitShare tmp = random_shares[i] * open_type(root.sqrRoot().invert()) + one;
        bits.push_back(tmp >> 1);
    }
    bit_MC->Check(bit_proc->P);
}

template<class T>
size_t Spdz2kPrep<T>::data_sent()
{
    size_t res = MascotPrep<T>::data_sent();
    if (bit_prep)
        res += bit_prep->data_sent();
    return res;
}
