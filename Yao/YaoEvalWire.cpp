/*
 * YaoEvalWire.cpp
 *
 */

#include "YaoEvalWire.h"
#include "YaoGate.h"
#include "YaoEvaluator.h"
#include "BMR/prf.h"
#include "BMR/common.h"

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

void YaoEvalWire::andrs(GC::Processor<GC::Secret<YaoEvalWire> >& processor,
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
		const Key& right_key = processor.S[args[i + 3]].get_reg(0).key;
		for (auto& left_wire: processor.S[args[i + 2]].get_regs())
		{
			long counter = ++evaluator.counter;
			labels[i_label++] = YaoGate::E_input(left_wire.key, right_key,
					counter);
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
		YaoEvalWire& right_wire = processor.S[args[i + 3]].get_reg(0);
		auto& out = processor.S[args[i + 1]];
		out.resize_regs(args[i]);
		int n = args[i];
		for (int k = 0; k < n; k++)
		{
			auto& left_wire = processor.S[args[i + 2]].get_reg(k);
			YaoGate gate;
			evaluator.load_gate(gate);
			gate.eval(out.get_reg(k), hashes[j++],
					gate.get_entry(left_wire.external, right_wire.external));
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
	cout << "output " << res << endl;
#endif
	return res;
}

void YaoEvalWire::set(const Key& key)
{
	this->key = key;
	external = key.get_signal();
}
