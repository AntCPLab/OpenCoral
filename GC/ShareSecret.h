/*
 * ReplicatedSecret.h
 *
 */

#ifndef GC_SHARESECRET_H_
#define GC_SHARESECRET_H_

#include <vector>
using namespace std;

#include "GC/Memory.h"
#include "GC/Clear.h"
#include "GC/Access.h"
#include "GC/ArgTuples.h"
#include "Math/FixedVec.h"
#include "Math/BitVec.h"
#include "Tools/SwitchableOutput.h"
#include "Protocols/Replicated.h"
#include "Protocols/ReplicatedMC.h"
#include "Processor/DummyProtocol.h"

namespace GC
{

template <class T>
class Processor;

template <class T>
class Thread;

template <class T>
class Machine;

template<class U>
class ShareSecret
{
public:
    static SwitchableOutput out;

    static void store_clear_in_dynamic(Memory<U>& mem,
            const vector<ClearWriteAccess>& accesses);

    static void load(vector< ReadAccess<U> >& accesses, const Memory<U>& mem);
    static void store(Memory<U>& mem, vector< WriteAccess<U> >& accesses);

    static void andrs(Processor<U>& processor, const vector<int>& args)
    { and_(processor, args, true); }
    static void ands(Processor<U>& processor, const vector<int>& args)
    { and_(processor, args, false); }
    static void and_(Processor<U>& processor, const vector<int>& args, bool repeat);
    static void inputb(Processor<U>& processor, const vector<int>& args);

    static void convcbit(Integer& dest, const Clear& source) { dest = source; }

    static BitVec get_mask(int n) { return n >= 64 ? -1 : ((1L << n) - 1); }

    void check_length(int n, const Integer& x);

    void random_bit();
};

template<class U>
class ReplicatedSecret : public FixedVec<BitVec, 2>, public ShareSecret<U>
{
    typedef FixedVec<BitVec, 2> super;

public:
    typedef BitVec clear;
    typedef BitVec open_type;
    typedef BitVec mac_type;
    typedef BitVec mac_key_type;

    typedef ReplicatedBase Protocol;

    static const int N_BITS = clear::N_BITS;

    static const bool dishonest_majority = false;
    static const bool needs_ot = false;

    static string type_string() { return "replicated secret"; }
    static string phase_name() { return "Replicated computation"; }

    static int default_length;

    static void trans(Processor<U>& processor, int n_outputs,
            const vector<int>& args);

    ReplicatedSecret() {}
    template <class T>
    ReplicatedSecret(const T& other) : super(other) {}

    void load_clear(int n, const Integer& x);

    void bitcom(Memory<U>& S, const vector<int>& regs);
    void bitdec(Memory<U>& S, const vector<int>& regs) const;

    BitVec local_mul(const ReplicatedSecret& other) const;

    void xor_(int n, const ReplicatedSecret& x, const ReplicatedSecret& y)
    { *this = x ^ y; (void)n; }

    void reveal(size_t n_bits, Clear& x);
};

class SemiHonestRepPrep;

class SemiHonestRepSecret : public ReplicatedSecret<SemiHonestRepSecret>
{
    typedef ReplicatedSecret<SemiHonestRepSecret> super;

public:
    typedef Memory<SemiHonestRepSecret> DynamicMemory;

    typedef ReplicatedMC<SemiHonestRepSecret> MC;
    typedef Replicated<SemiHonestRepSecret> Protocol;
    typedef MC MAC_Check;
    typedef SemiHonestRepPrep LivePrep;
    typedef ReplicatedInput<SemiHonestRepSecret> Input;

    static MC* new_mc(Machine<SemiHonestRepSecret>& _) { (void) _; return new MC; }

    SemiHonestRepSecret() {}
    template<class T>
    SemiHonestRepSecret(const T& other) : super(other) {}
};

}

#endif /* GC_SHARESECRET_H_ */
