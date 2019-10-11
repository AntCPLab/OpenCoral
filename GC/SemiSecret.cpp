/*
 * SemiSecret.cpp
 *
 */

#include "GC/ShareParty.h"
#include "SemiSecret.h"

#include "GC/ShareSecret.hpp"
#include "Protocols/MAC_Check_Base.hpp"

namespace GC
{

void SemiSecret::trans(Processor<SemiSecret>& processor, int n_outputs,
        const vector<int>& args)
{
    square64 square;
    for (size_t i = n_outputs; i < args.size(); i++)
        square.rows[i - n_outputs] = processor.S[args[i]].get();
    square.transpose(args.size() - n_outputs, n_outputs);
    for (int i = 0; i < n_outputs; i++)
        processor.S[args[i]] = square.rows[i];
}

void SemiSecret::load_clear(int n, const Integer& x)
{
    check_length(n, x);
    *this = constant(x, Thread<SemiSecret>::s().P->my_num());
}

void SemiSecret::bitcom(Memory<SemiSecret>& S, const vector<int>& regs)
{
    *this = 0;
    for (unsigned int i = 0; i < regs.size(); i++)
        *this ^= (S[regs[i]] << i);
}

void SemiSecret::bitdec(Memory<SemiSecret>& S,
        const vector<int>& regs) const
{
    for (unsigned int i = 0; i < regs.size(); i++)
        S[regs[i]] = (*this >> i) & 1;
}

void SemiSecret::reveal(size_t n_bits, Clear& x)
{
    auto& thread = Thread<SemiSecret>::s();
    x = thread.MC->POpen(*this, *thread.P).mask(n_bits);
}

} /* namespace GC */
