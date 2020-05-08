/*
 * Secret.h
 *
 */

#ifndef GC_FAKESECRET_H_
#define GC_FAKESECRET_H_

#include "GC/Clear.h"
#include "GC/Memory.h"
#include "GC/Access.h"
#include "GC/ArgTuples.h"

#include "Math/gf2nlong.h"

#include "Processor/DummyProtocol.h"

#include <random>
#include <fstream>

namespace GC
{

template <class T>
class Processor;
template <class T>
class Machine;

class FakeSecret
{
    __uint128_t a;

public:
    typedef FakeSecret DynamicType;
    typedef Memory<FakeSecret> DynamicMemory;

    static string type_string() { return "fake secret"; }
    static string phase_name() { return "Faking"; }

    static int default_length;

    typedef ostream& out_type;
    static ostream& out;

    static void store_clear_in_dynamic(Memory<DynamicType>& mem,
    		const vector<GC::ClearWriteAccess>& accesses);

    static void load(vector< ReadAccess<FakeSecret> >& accesses, const Memory<FakeSecret>& mem);
    static void store(Memory<FakeSecret>& mem, vector< WriteAccess<FakeSecret> >& accesses);

    template <class T>
    static void andrs(T& processor, const vector<int>& args)
    { processor.andrs(args); }
    static void ands(GC::Processor<FakeSecret>& processor, const vector<int>& regs);
    template <class T>
    static void xors(GC::Processor<T>& processor, const vector<int>& regs)
    { processor.xors(regs); }
    template <class T>
    static void inputb(T& processor, const vector<int>& args)
    { processor.input(args); }
    template <class T>
    static void reveal_inst(T& processor, const vector<int>& args)
    { processor.reveal(args); }

    static void trans(Processor<FakeSecret>& processor, int n_inputs,
            const vector<int>& args);

    template <class T>
    static void convcbit(Integer& dest, const Clear& source, T&) { dest = source; }

    static FakeSecret input(GC::Processor<FakeSecret>& processor, const InputArgs& args);
    static FakeSecret input(int from, const int128& input, int n_bits);

    FakeSecret() : a(0) {}
    FakeSecret(const Integer& x) : a(x.get()) {}
    FakeSecret(__uint128_t x) : a(x) {}

    __uint128_t operator>>(const FakeSecret& other) const { return a >> other.a; }
    __uint128_t operator<<(const FakeSecret& other) const { return a << other.a; }

    __uint128_t operator^=(const FakeSecret& other) { return a ^= other.a; }

    void load_clear(int n, const Integer& x);
    template <class T>
    void load(int n, const Memory<T>& mem, size_t address) { load(n, mem[address]); }
    template <class T>
    void store(Memory<T>& mem, size_t address) { mem[address] = *this; }

    void bitcom(Memory<FakeSecret>& S, const vector<int>& regs);
    void bitdec(Memory<FakeSecret>& S, const vector<int>& regs) const;

    template <class T>
    void xor_(int n, const FakeSecret& x, const T& y) { (void)n; a = x.a ^ y.a; }
    void and_(int n, const FakeSecret& x, const FakeSecret& y, bool repeat);
    void andrs(int n, const FakeSecret& x, const FakeSecret& y) { (void)n; a = x.a * y.a; }

    void invert(int, const FakeSecret& x) { *this = ~x.a; }

    void random_bit() { a = random() % 2; }

    void reveal(int n_bits, Clear& x) { (void) n_bits; x = a; }

    int size() { return -1; }
};

} /* namespace GC */

#endif /* GC_FAKESECRET_H_ */
