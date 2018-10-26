/*
 * ReplicatedSecret.h
 *
 */

#ifndef GC_REPLICATEDSECRET_H_
#define GC_REPLICATEDSECRET_H_

#include <vector>
using namespace std;

#include "GC/Memory.h"
#include "GC/Clear.h"
#include "GC/Access.h"
#include "Math/FixedVec.h"
#include "Math/BitVec.h"
#include "Tools/SwitchableOutput.h"
#include "Processor/Replicated.h"

namespace GC
{

template <class T>
class Processor;

template <class T>
class Thread;

class ReplicatedSecret : public FixedVec<BitVec, 2>
{
    typedef FixedVec<BitVec, 2> super;

public:
    typedef ReplicatedSecret DynamicType;

    typedef BitVec clear;

    typedef ReplicatedMC<ReplicatedSecret> MC;
    typedef void Inp;
    typedef void PO;
    typedef ReplicatedBase Protocol;

    static string type_string() { return "replicated secret"; }
    static string phase_name() { return "Replicated computation"; }

    static int default_length;
    static SwitchableOutput out;

    static void store_clear_in_dynamic(Memory<DynamicType>& mem,
            const vector<ClearWriteAccess>& accesses);

    static void load(vector< ReadAccess<ReplicatedSecret> >& accesses, const Memory<ReplicatedSecret>& mem);
    static void store(Memory<ReplicatedSecret>& mem, vector< WriteAccess<ReplicatedSecret> >& accesses);

    static void andrs(Processor<ReplicatedSecret>& processor, const vector<int>& args)
    { and_(processor, args, true); }
    static void ands(Processor<ReplicatedSecret>& processor, const vector<int>& args)
    { and_(processor, args, false); }
    static void and_(Processor<ReplicatedSecret>& processor, const vector<int>& args, bool repeat);
    static void inputb(Processor<ReplicatedSecret>& processor, const vector<int>& args);

    static void trans(Processor<ReplicatedSecret>& processor, int n_outputs,
            const vector<int>& args);

    static BitVec get_mask(int n) { return n >= 64 ? -1 : ((1L << n) - 1); }

    static ReplicatedSecret input(int from, Processor<ReplicatedSecret>& processor, int n_bits);
    void prepare_input(vector<octetStream>& os, long input, int n_bits, PRNG& secure_prng);
    void finalize_input(Thread<ReplicatedSecret>& party, octetStream& o, int from, int n_bits);

    ReplicatedSecret() {}
    template <class T>
    ReplicatedSecret(const T& other) : super(other) {}

    void load(int n, const Integer& x);

    void bitcom(Memory<ReplicatedSecret>& S, const vector<int>& regs);
    void bitdec(Memory<ReplicatedSecret>& S, const vector<int>& regs) const;

    void xor_(int n, const ReplicatedSecret& x, const ReplicatedSecret& y)
    { *this = x ^ y; (void)n; }
    void and_(int n, const ReplicatedSecret& x, const ReplicatedSecret& y, bool repeat);
    void andrs(int n, const ReplicatedSecret& x, const ReplicatedSecret& y);
    void prepare_and(vector<octetStream>& os, int n,
            const ReplicatedSecret& x, const ReplicatedSecret& y,
            Thread<ReplicatedSecret>& party, bool repeat);
    void finalize_andrs(vector<octetStream>& os, int n);

    void reveal(Clear& x);

    void random_bit();
};

}

#endif /* GC_REPLICATEDSECRET_H_ */
