/*
 * YaoEvalWire.cpp
 *
 */

#include "config.h"
#include "YaoEvalWire.h"
#include "YaoGate.h"
#include "YaoEvaluator.h"
#include "BMR/prf.h"
#include "BMR/common.h"
#include "GC/ArgTuples.h"

#include "GC/Processor.hpp"
#include "GC/Secret.hpp"
#include "GC/Thread.hpp"
#include "YaoCommon.hpp"

ostream& YaoEvalWire::out = cout;

void YaoEvalWire::random()
{
	set(0);
}

void YaoEvalWire::public_input(bool value)
{
	(void)value;
	set(0);
}

template<bool repeat>
void YaoEvalWire::and_(GC::Processor<GC::Secret<YaoEvalWire> >& processor,
		const vector<int>& args)
{
	YaoEvaluator& party = YaoEvaluator::s();
	int total = processor.check_args(args, 4);
	int threshold = 1024;
	if (total < threshold)
	{
		// run in single thread
		and_singlethread<repeat>(processor, args, total);
		return;
	}

	processor.complexity += total;
	int i_thread = 0, start = 0;
	for (auto& x : party.get_splits(args, threshold, total))
	{
		auto i_gate = x[0];
		auto end = x[1];
		YaoGate* gate = (YaoGate*) party.gates.consume(
				i_gate * sizeof(YaoGate));
		party.jobs[i_thread++]->dispatch(YAO_AND_JOB, processor, args, start,
				end, i_gate, gate, party.get_gate_id(), repeat);
		party.counter += i_gate;
		start = end;
	}
	party.wait(i_thread);
}

template<bool repeat>
void YaoEvalWire::and_singlethread(GC::Processor<GC::Secret<YaoEvalWire> >& processor,
		const vector<int>& args, int total_ands)
{
	if (total_ands < 10)
		return processor.and_(args, repeat);
	processor.complexity += total_ands;
	size_t n_args = args.size();
	YaoEvaluator& party = YaoEvaluator::s();
	YaoGate* gate = (YaoGate*) party.gates.consume(total_ands * sizeof(YaoGate));
	long counter = party.get_gate_id();
	map<string, Timer> timers;
	SeededPRNG prng;
	and_(processor.S, args, 0, n_args, total_ands, gate, counter,
			prng, timers, repeat, party);
	party.counter += counter - party.get_gate_id();
}

void YaoEvalWire::and_(GC::Memory<GC::Secret<YaoEvalWire> >& S,
		const vector<int>& args, size_t start, size_t end, size_t,
		YaoGate* gates, long& gate_id, PRNG&, map<string, Timer>&,
		bool repeat, YaoEvaluator& evaluator)
{
	int dl = GC::Secret<YaoEvalWire>::default_length;
	MMO& mmo = evaluator.mmo;
	for (auto it = args.begin() + start; it < args.begin() + end; it += 4)
	{
		if (*it == 1)
		{
			Key label[YaoGate::N_EVAL_HASHES];
			Key hash[YaoGate::N_EVAL_HASHES];
			gate_id++;
			YaoGate::eval_inputs(label,
					S[*(it + 2)].get_reg(0).key(),
					S[*(it + 3)].get_reg(0).key(),
					gate_id);
			mmo.hash<YaoGate::N_EVAL_HASHES>(hash, label);
			auto& out = S[*(it + 1)];
			out.resize_regs(1);
			YaoGate& gate = *gates;
			gates++;
			gate.eval(out.get_reg(0), hash, S[*(it + 2)].get_reg(0),
					S[*(it + 3)].get_reg(0));
		}
		else
		{
			int n_units = DIV_CEIL(*it, dl);
			for (int j = 0; j < n_units; j++)
			{
				auto& left = S[*(it + 2) + j];
				auto& right = S[*(it + 3) + (repeat ? 0 : j)];
				auto& out = S[*(it + 1) + j];
				int n = min(dl, *it - j * dl);
				out.resize_regs(n);
				for (int k = 0; k < n; k++)
				{
					Key label[YaoGate::N_EVAL_HASHES];
					Key hash[YaoGate::N_EVAL_HASHES];
					auto& left_wire = left.get_reg(k);
					auto& right_key = right.get_reg(repeat ? 0 : k).key();
					gate_id++;
					YaoGate::eval_inputs(label, left_wire.key(), right_key,
							gate_id);
					mmo.hash<YaoGate::N_EVAL_HASHES>(hash, label);
					auto& right_wire = right.get_reg(repeat ? 0 : k);
					YaoGate& gate = *gates;
					gates++;
					gate.eval(out.get_reg(k), hash, left_wire, right_wire);
				}
			}
		}
	}
}

void YaoEvalWire::inputb(GC::Processor<GC::Secret<YaoEvalWire> >& processor,
        const vector<int>& args)
{
	InputArgList a(args);
	BitVector inputs;
	inputs.resize(0);
	auto& evaluator = YaoEvaluator::s();
	bool interactive = evaluator.n_interactive_inputs_from_me(a) > 0;

	for (auto x : a)
	{
		auto& dest = processor.S[x.dest];
		dest.resize_regs(x.n_bits);
		if (x.from == 0)
		{
			for (auto& reg : dest.get_regs())
			{
				reg.set(0);
			}
		}
		else
		{
			long long input = processor.get_input(x.params, interactive);
			size_t start = inputs.size();
			inputs.resize(start + x.n_bits);
			for (int i = 0; i < x.n_bits; i++)
				inputs.set_bit(start + i, (input >> i) & 1);
		}
	}

	if (interactive)
	    cout << "Thank you" << endl;

	evaluator.ot_ext.extend_correlated(inputs.size(), inputs);
	octetStream os;
	evaluator.player.receive(os);
	int i_bit = 0;

	for (auto x : a)
	{
		if (x.from == 1)
		{
			for (auto& reg : processor.S[x.dest].get_regs())
			{
				Key key;
				os.unserialize(key);
                reg.set(key ^ evaluator.ot_ext.receiverOutputMatrix[i_bit],
                        inputs.get_bit(i_bit));
				i_bit++;
			}
		}
	}
}

void YaoEvalWire::op(const YaoEvalWire& left, const YaoEvalWire& right,
		Function func)
{
    (void)func;
	YaoGate gate;
	YaoEvaluator::s().load_gate(gate);
	YaoEvaluator::s().counter++;
	gate.eval(*this, left, right);
}

bool YaoEvalWire::get_output()
{
	YaoEvaluator::s().taint();
	bool res = external() ^ YaoEvaluator::s().output_masks.pop_front();
#ifdef DEBUG
    cout << "output " << res << " mask " << (external() ^ res) << " external() "
            << external() << endl;
#endif
	return res;
}

void YaoEvalWire::set(const Key& key)
{
	this->key_ = key;
}

void YaoEvalWire::set(Key key, bool external)
{
	assert(key.get_signal() == external);
	set(key);
}

void YaoEvalWire::convcbit(Integer& dest, const GC::Clear& source,
		GC::Processor<GC::Secret<YaoEvalWire>>&)
{
	auto& evaluator = YaoEvaluator::s();
	dest = source;
	evaluator.P->send_long(0, source.get());
	throw needs_cleaning();
}

template void YaoEvalWire::and_<false>(
        GC::Processor<GC::Secret<YaoEvalWire> >& processor,
        const vector<int>& args);
template void YaoEvalWire::and_<true>(
        GC::Processor<GC::Secret<YaoEvalWire> >& processor,
        const vector<int>& args);
