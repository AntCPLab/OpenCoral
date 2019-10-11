/*
 * EvalSecret.h
 *
 */

#ifndef GC_SECRET_H_
#define GC_SECRET_H_

#include "GC/config.h"
#include "BMR/config.h"
#include "BMR/common.h"

#include "GC/Clear.h"
#include "GC/Memory.h"
#include "GC/Access.h"
#include "GC/ArgTuples.h"

#include "Math/gf2nlong.h"

#include "Processor/DummyProtocol.h"

#include <fstream>

namespace GC
{

template<class T>
class Secret;

template <class T>
inline void XOR(T& res, const T& left, const T& right)
{
#ifdef FREE_XOR
    Secret<T>::cast(res).XOR(Secret<T>::cast(left), Secret<T>::cast(right));
#else
    Secret<T>::cast(res).op(Secret<T>::cast(left), Secret<T>::cast(right), 0x0110);
#endif
}

class AuthValue
{
public:
    static string type_string() { return "authenticated value"; }
    word share;
    int128 mac;
    AuthValue() : share(0), mac(0) {}
    void assign(const word& value, const int128& mac_key, bool first_player);
    void check(const word& mac_key) const;
    friend ostream& operator<<(ostream& o, const AuthValue& auth_value);
};

class Mask
{
public:
    word share;
    int128 mac;
    Mask() : share(0) {}
};

template<class T> class Processor;
template<class T> class Machine;

template <class T>
class Secret
{
    CheckVector<T> registers;

    T& get_new_reg();

public:
    typedef typename T::DynamicMemory DynamicMemory;

    // dummy
    typedef DummyMC MC;
    typedef DummyProtocol Protocol;

    static MC* new_mc(Machine<Secret>& _) { (void) _; return new MC; }

    static string type_string() { return "evaluation secret"; }
    static string phase_name() { return T::name(); }

    static int default_length;

    static typename T::out_type out;

    static const bool needs_ot = false;

    static T& cast(T& reg) { return *reinterpret_cast<T*>(&reg); }
    static const T& cast(const T& reg) { return *reinterpret_cast<const T*>(&reg); }

    static Secret<T> input(party_id_t from, const int128& input, int n_bits = -1);
    static Secret<T> input(Processor<Secret<T>>& processor, const InputArgs& args);
    void random(int n_bits, int128 share);
    void random_bit();
    static Secret<T> reconstruct(const int128& x, int length);
    template <class U>
    static void store_clear_in_dynamic(U& mem, const vector<ClearWriteAccess>& accesses)
    { T::store_clear_in_dynamic(mem, accesses); }
    void store(Memory<AuthValue>& mem, size_t address);
    static Secret<T> carryless_mult(const Secret<T>& x, const Secret<T>& y);
    static void output(T& reg);

    template<class U, class V>
    static void load(vector< ReadAccess<V> >& accesses, const U& mem);
    template<class U, class V>
    static void store(U& mem, vector< WriteAccess<V> >& accesses);

    template<class U>
    static void andrs(Processor<U>& processor, const vector<int>& args)
    { T::andrs(processor, args); }
    template<class U>
    static void ands(Processor<U>& processor, const vector<int>& args)
    { T::ands(processor, args); }
    template<class U>
    static void inputb(Processor<U>& processor, const vector<int>& args)
    { T::inputb(processor, args); }

    template<class U>
    static void trans(Processor<U>& processor, int n_inputs, const vector<int>& args);

    static void convcbit(Integer& dest, const Clear& source) { T::convcbit(dest, source); }

    Secret();
    Secret(const Integer& x) { *this = x; }

    void load_clear(int n, const Integer& x);
    void operator=(const Integer& x) { load_clear(default_length, x); }
    void load(int n, const Memory<AuthValue>& mem, size_t address);

    Secret<T> operator<<(int i);
    Secret<T> operator>>(int i);

    template<class U>
    void bitcom(Memory<U>& S, const vector<int>& regs);
    template<class U>
    void bitdec(Memory<U>& S, const vector<int>& regs) const;

    Secret<T> operator+(const Secret<T> x) const;
    Secret<T>& operator+=(const Secret<T> x) { *this = *this + x; return *this; }

    void xor_(int n, const Secret<T>& x, const Secret<T>& y)
    {
        resize_regs(n);
        for (int i = 0; i < n; i++)
            XOR<T>(registers[i], x.get_reg(i), y.get_reg(i));
    }
    void and_(int n, const Secret<T>& x, const Secret<T>& y, bool repeat);
    void andrs(int n, const Secret<T>& x, const Secret<T>& y) { and_(n, x, y, true); }

    template <class U>
    void reveal(size_t n_bits, U& x);

    int size() const { return registers.size(); }
    CheckVector<T>& get_regs() { return registers; }
    const CheckVector<T>& get_regs() const { return registers; }

    const T& get_reg(int i) const { return *reinterpret_cast<const T*>(&registers.at(i)); }
    T& get_reg(int i) { return *reinterpret_cast<T*>(&registers.at(i)); }
    void resize_regs(int n);
};

template <class T>
int Secret<T>::default_length = 128;

template <class T>
typename T::out_type Secret<T>::out = T::out;

template <class T>
inline ostream& operator<<(ostream& o, Secret<T>& secret)
{
	o << "(" << secret.size() << " secret bits)";
	return o;
}

} /* namespace GC */

#endif /* GC_SECRET_H_ */
