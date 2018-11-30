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
	int total_ands = processor.check_args(args, 4);
	if (total_ands < 10)
		return processor.andrs(args);
	processor.complexity += total_ands;
	Key* labels;
	Key* hashes;
	vector<Key> label_vec, hash_vec;
	size_t n_hashes = total_ands;
	Key label_arr[1000], hash_arr[1000];
	if (total_ands < 1000)
	{
		labels = label_arr;
		hashes = hash_arr;
	}
	else
	{
		label_vec.resize(n_hashes);
		hash_vec.resize(n_hashes);
		labels = label_vec.data();
		hashes = hash_vec.data();
	}
	size_t i_label = 0;
	size_t n_args = args.size();
	auto& evaluator = YaoEvaluator::s();
	for (size_t i = 0; i < n_args; i += 4)
	{
		auto& left = processor.S[args[i + 2]];
		auto& right = processor.S[args[i + 3]];

		for (int k = 0; k < args[i]; k++)
		{
			auto& left_wire = left.get_reg(k);
			auto& right_key = right.get_reg(repeat ? 0 : k).key;
			evaluator.counter++;
			labels[i_label++] = YaoGate::E_input(left_wire.key, right_key,
					evaluator.get_gate_id());
		}
	}
	MMO& mmo = evaluator.mmo;
	size_t i;
	for (i = 0; i + 8 <= n_hashes; i += 8)
		mmo.hash<8>(&hashes[i], &labels[i]);
	for (; i < n_hashes; i++)
		hashes[i] = mmo.hash(labels[i]);
	size_t j = 0;
	for (size_t i = 0; i < n_args; i += 4)
	{
		auto& left = processor.S[args[i + 2]];
		auto& right = processor.S[args[i + 3]];
		auto& out = processor.S[args[i + 1]];
		out.resize_regs(args[i]);
		int n = args[i];

		for (int k = 0; k < n; k++)
		{
			auto& right_wire = right.get_reg(repeat ? 0 : k);
			auto& left_wire = left.get_reg(k);
			YaoGate gate;
			evaluator.load_gate(gate);
			gate.eval(out.get_reg(k), hashes[j++],
					gate.get_entry(left_wire.external, right_wire.external));
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
			long long input = processor.get_input(x.n_bits, interactive);
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

void YaoEvalWire::XOR(const YaoEvalWire& left, const YaoEvalWire& right)
{
	external = left.external ^ right.external;
	key = left.key ^ right.key;
}

bool YaoEvalWire::get_output()
{
	bool res = external ^ YaoEvaluator::s().output_masks.pop_front();
#ifdef DEBUG
    cout << "output " << res << " mask " << (external ^ res) << " external "
            << external << endl;
#endif
	return res;
}

void YaoEvalWire::set(const Key& key)
{
	this->key = key;
	external = key.get_signal();
}

void YaoEvalWire::set(Key key, bool external)
{
	key.set_signal(external);
	set(key);
}

template void YaoEvalWire::and_<false>(
        GC::Processor<GC::Secret<YaoEvalWire> >& processor,
        const vector<int>& args);
template void YaoEvalWire::and_<true>(
        GC::Processor<GC::Secret<YaoEvalWire> >& processor,
        const vector<int>& args);
