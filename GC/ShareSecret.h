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
#include "GC/NoShare.h"
#include "Math/FixedVec.h"
#include "Math/BitVec.h"
#include "Tools/SwitchableOutput.h"
#include "Protocols/Replicated.h"
#include "Protocols/ReplicatedMC.h"
#include "Processor/DummyProtocol.h"
#include "Processor/ProcessorBase.h"

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
    typedef Memory<U> DynamicMemory;
    typedef SwitchableOutput out_type;

    static const bool is_real = true;
    static const bool actual_inputs = true;

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
    static void xors(Processor<U>& processor, const vector<int>& args);
    static void inputb(Processor<U>& processor, const vector<int>& args)
    { inputb(processor, processor, args); }
    static void inputb(Processor<U>& processor, ProcessorBase& input_processor,
            const vector<int>& args);
    static void inputbvec(Processor<U>& processor, ProcessorBase& input_processor,
            const vector<int>& args);
    static void reveal_inst(Processor<U>& processor, const vector<int>& args);

    template<class T>
    static void convcbit(Integer& dest, const Clear& source, T&) { dest = source; }

    static BitVec get_mask(int n) { return n >= 64 ? -1 : ((1L << n) - 1); }

    void check_length(int n, const Integer& x);

    void invert(int n, const U& x);

    void random_bit();

    template<class T>
    void my_input(T& inputter, BitVec value, int n_bits);
    template<class T>
    void other_input(T& inputter, int from, int n_bits = 1);
    template<class T>
    void finalize_input(T& inputter, int from, int n_bits);
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

    typedef NoShare bit_type;

    static const int N_BITS = clear::N_BITS;

    static const bool dishonest_majority = false;
    static const bool variable_players = false;
    static const bool needs_ot = false;

    static string type_string() { return "replicated secret"; }
    static string phase_name() { return "Replicated computation"; }

    static const int default_length =  8 * sizeof(typename ReplicatedSecret<U>::value_type);

    static int threshold(int)
    {
        return 1;
    }

    static void trans(Processor<U>& processor, int n_outputs,
            const vector<int>& args);

    template<class T>
    static void generate_mac_key(mac_key_type, T)
    {
    }

    static void read_or_generate_mac_key(string, const Names&, mac_key_type) {}

    static ReplicatedSecret constant(const clear& value, int my_num, mac_key_type)
    {
      ReplicatedSecret res;
      if (my_num < 2)
        res[my_num] = value;
      return res;
    }

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

    ReplicatedSecret operator&(const Clear& other)
    { return super::operator&(BitVec(other)); }

    ReplicatedSecret lsb()
    { return *this & 1; }

    ReplicatedSecret get_bit(int i)
    { return (*this >> i) & 1; }
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

    typedef SemiHonestRepSecret part_type;
    typedef SemiHonestRepSecret small_type;
    typedef SemiHonestRepSecret whole_type;

    static const bool expensive_triples = false;

    static MC* new_mc(mac_key_type) { return new MC; }

    SemiHonestRepSecret() {}
    template<class T>
    SemiHonestRepSecret(const T& other) : super(other) {}
};

}

#endif /* GC_SHARESECRET_H_ */
