/*
 * Secret.cpp
 *
 */

#include <GC/FakeSecret.h>
#include "GC/Processor.h"
#include "GC/square64.h"

#include "GC/Processor.hpp"

namespace GC
{

int FakeSecret::default_length = 128;

ostream& FakeSecret::out = cout;

void FakeSecret::load_clear(int n, const Integer& x)
{
	if ((size_t)n < 8 * sizeof(x) and abs(x.get()) >= (1LL << n))
		throw out_of_range("public value too long");
	*this = x;
}

void FakeSecret::bitcom(Memory<FakeSecret>& S, const vector<int>& regs)
{
    *this = 0;
    for (unsigned int i = 0; i < regs.size(); i++)
        *this ^= (S[regs[i]] << i);
}

void FakeSecret::bitdec(Memory<FakeSecret>& S, const vector<int>& regs) const
{
    for (unsigned int i = 0; i < regs.size(); i++)
        S[regs[i]] = (*this >> i) & 1;
}

void FakeSecret::load(vector<ReadAccess<FakeSecret> >& accesses,
        const Memory<FakeSecret>& mem)
{
    for (auto access : accesses)
        access.dest = mem[access.address];
}

void FakeSecret::store(Memory<FakeSecret>& mem,
        vector<WriteAccess<FakeSecret> >& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.source;
}

void FakeSecret::store_clear_in_dynamic(Memory<DynamicType>& mem,
		const vector<GC::ClearWriteAccess>& accesses)
{
	for (auto access : accesses)
		mem[access.address] = access.value;
}

void FakeSecret::ands(Processor<FakeSecret>& processor,
        const vector<int>& regs)
{
	processor.check_args(regs, 4);
	for (size_t i = 0; i < regs.size(); i += 4)
		processor.S[regs[i + 1]] = processor.S[regs[i + 2]].a & processor.S[regs[i + 3]].a;
}


void FakeSecret::trans(Processor<FakeSecret>& processor, int n_outputs,
        const vector<int>& args)
{
	square64 square;
	for (size_t i = n_outputs; i < args.size(); i++)
		square.rows[i - n_outputs] = processor.S[args[i]].a;
	square.transpose(args.size() - n_outputs, n_outputs);
	for (int i = 0; i < n_outputs; i++)
		processor.S[args[i]] = square.rows[i];
}

FakeSecret FakeSecret::input(GC::Processor<FakeSecret>& processor, const InputArgs& args)
{
	return input(args.from, processor.get_input(args.params), args.n_bits);
}

FakeSecret FakeSecret::input(int from, const int128& input, int n_bits)
{
	(void)from;
	(void)n_bits;
	FakeSecret res;
	res.a = ((__uint128_t)input.get_upper() << 64) + input.get_lower();
	if (res.a > ((__uint128_t)1 << n_bits))
		throw out_of_range("input too large");
	return res;
}

void FakeSecret::and_(int n, const FakeSecret& x, const FakeSecret& y,
        bool repeat)
{
	if (repeat)
		return andrs(n, x, y);
	else
		throw runtime_error("call static FakeSecret::ands()");
}

} /* namespace GC */
