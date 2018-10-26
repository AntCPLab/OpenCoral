/*
 * Secret.cpp
 *
 */

#include "BMR/CommonParty.h"
#include "BMR/Register_inline.h"

#include "GC/Machine.hpp"
#include "GC/Processor.hpp"
#include "GC/Secret.hpp"
#include "GC/Thread.hpp"
#include "GC/ThreadMaster.hpp"

namespace GC
{

template <class T>
Secret<T> Secret<T>::reconstruct(const int128& x, int length)
{
    Secret<T> res;
    for (int i = 0; i < CommonParty::singleton->get_n_parties(); i++)
    {
        Secret<T> tmp = res;
        Secret<T> share = input(i + 1, x, length);
        res = share + tmp;
#ifdef DEBUG_DYNAMIC
        int128 a,b,c;
        tmp.reveal(a);
        share.reveal(b);
        res.reveal(c);
        cout << hex << c << "(" << dec << res.size() << ") = " << hex << a
                << "(" << dec << tmp.size() << ")" << " ^ " << hex << b << "("
                << dec << share.size() << ") (" << dec << x << ", " << dec
                << length << ")" << endl;
#endif
    }
    return res;
    if ((size_t)length != res.registers.size())
    {
        cout << length << " " << res.registers.size() << endl;
        throw runtime_error("wrong bit length in reconstruct()");
    }
}

template <class T>
void Secret<T>::store(Memory<AuthValue>& mem, size_t address)
{
    AuthValue& dest = mem[address];
    Secret<T> mac_key = reconstruct(CommonParty::s().get_mac_key().get(), default_length);
    Secret<T> mac, mask, mac_mask;
    mac = carryless_mult(*this, mac_key);
    GC::Mask mask_share;
    int length = registers.size();
    int mac_length = mac.registers.size();
    T::get_dyn_mask(mask_share, length, mac_length);
    mask.random(length, mask_share.share);
    mac_mask.random(mac_length, mask_share.mac);
    word masked;
    int128 masked_mac;
    (*this + mask).reveal(masked);
    (mac + mac_mask).reveal(masked_mac);
#ifdef DEBUG_DYNAMIC
    word a,b;
    int128 c,d;
    reveal(a);
    mask.reveal(b);
    mac.reveal(c);
    mac_mask.reveal(d);
    cout << masked << " = " << a << " ^ " << b << endl;
    cout << masked_mac << " = " << c << " ^ " << d << endl;
#endif
    T::unmask(dest, mask_share.share, mask_share.mac, masked, masked_mac);
}

template <class T>
void Secret<T>::load(int n, const Memory<AuthValue>& mem, size_t address)
{
    (void)n;
    const AuthValue& x = mem[address];
    *this = reconstruct(x.share, default_length);
    Secret<T> mac, check_mac, mac_key;
    mac = reconstruct(x.mac, 2 * default_length);
    mac_key = reconstruct(CommonParty::s().get_mac_key().get(), default_length);
    check_mac = carryless_mult(*this, mac_key);
    int128 result;
    (mac + check_mac).reveal(result);
#ifdef DEBUG_DYNAMIC
    cout << "loading " << hex << x.share << " " << x.mac << endl;
    int128 a;
    mac.reveal(a);
    word b;
    reveal(b);
    cout << "stored value " << hex << b << " mac " << a << endl;
#endif
    T::check(result, x.share, x.mac);
}

template class Secret<EvalRegister>;
template class Secret<PRFRegister>;
template class Secret<GarbleRegister>;
template class Secret<RandomRegister>;

template void Secret<EvalRegister>::reveal(Clear& x);
template void Secret<PRFRegister>::reveal(Clear& x);
template void Secret<GarbleRegister>::reveal(Clear& x);
template void Secret<RandomRegister>::reveal(Clear& x);

template class Machine< Secret<PRFRegister> >;
template class Machine< Secret<EvalRegister> >;
template class Machine< Secret<GarbleRegister> >;
template class Machine< Secret<RandomRegister> >;

template class Processor< Secret<PRFRegister> >;
template class Processor< Secret<EvalRegister> >;
template class Processor< Secret<GarbleRegister> >;
template class Processor< Secret<RandomRegister> >;

template class Thread< Secret<PRFRegister> >;
template class Thread< Secret<EvalRegister> >;
template class Thread< Secret<GarbleRegister> >;
template class Thread< Secret<RandomRegister> >;

template class ThreadMaster< Secret<PRFRegister> >;
template class ThreadMaster< Secret<EvalRegister> >;
template class ThreadMaster< Secret<GarbleRegister> >;
template class ThreadMaster< Secret<RandomRegister> >;

}
